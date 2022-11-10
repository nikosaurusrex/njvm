namespace jit {
    using namespace llvm;
    using namespace llvm::orc;

#define STR_REF(x) StringRef((const char * ) x.data, x.length)
    struct LLVMJit {
        ExecutionSession ES;
        RTDyldObjectLinkingLayer ObjectLayer;
        IRCompileLayer CompileLayer;
        IRTransformLayer TransformLayer;
        DataLayout DL;
        MangleAndInterner Mangle;
        ThreadSafeContext Ctx;

        LLVMJit(JITTargetMachineBuilder JTMB, DataLayout DL) : ObjectLayer(ES,
        []() { return llvm::make_unique<SectionMemoryManager>(); }),
        CompileLayer(ES, ObjectLayer, ConcurrentIRCompiler(std::move(JTMB))),
        /* Transform: maybe optimize */
        TransformLayer(ES, CompileLayer, IRTransformLayer::identityTransform),
        DL(std::move(DL)), Mangle(ES, this->DL),
        Ctx(llvm::make_unique<LLVMContext>()) {
            ES.getMainJITDylib().setGenerator(
                    cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(DL)));
        }

        static Expected<std::unique_ptr<LLVMJit>> Create() {
            auto JTMB = JITTargetMachineBuilder::detectHost();
            if ( ! JTMB )
                return JTMB.takeError();

            auto DL = JTMB->getDefaultDataLayoutForTarget();
            if ( ! DL )
                return DL.takeError();

            return llvm::make_unique<LLVMJit>(std::move(*JTMB), std::move(*DL));
        }

        const DataLayout& getDataLayout() const { return DL; }
        LLVMContext& getContext() { return *Ctx.getContext(); }

        Error addModule(std::unique_ptr<Module> M) {
            return TransformLayer.add(ES.getMainJITDylib(), ThreadSafeModule(std::move(M), Ctx));
        }

        Expected<JITEvaluatedSymbol> lookup(StringRef Name) {
            return ES.lookup({&ES.getMainJITDylib()}, Mangle(Name.str()));
        }
    };

    struct Jit : Backend {
        LLVMContext context;
        std::unique_ptr<Module> module;
        TargetMachine *target_machine;
        IRBuilder<> *irb;
        Value **stack;
        Value **locals;

        Type *llty_i8;
        Type *llty_i32;
        Type *llty_void;

        Jit(Class *main_clazz) : Backend(main_clazz) {
            InitializeAllTargetInfos();
            InitializeAllTargets();
            InitializeAllTargetMCs();
            InitializeAllAsmParsers();
            InitializeAllAsmPrinters();

            module = make_unique<Module>("njit", context);
            irb = new IRBuilder<>(context);

            llty_i8 = Type::getInt8Ty(context);
            llty_i32 = Type::getInt32Ty(context);
            llty_void = Type::getVoidTy(context);
        }

        void run() override {
            for (u16 i = 0; i < clazz->methods_count; ++i) {
                convert_method(&clazz->methods[i]);
            }

            finalize();
        }

        void finalize() {
            std::string target_triple = llvm::sys::getDefaultTargetTriple();
            target_machine = EngineBuilder().selectTarget();

            module->setDataLayout(target_machine->createDataLayout());
            module->setTargetTriple(target_triple);

            // optimize();
            module->print(outs(), 0);

            auto llvm_jit_q = LLVMJit::Create();
            if (!llvm_jit_q){
                printf("Failed to initialize the JIT compiler. Terminating.");
                exit(1);
            }
            auto llvm_jit = std::move(*llvm_jit_q);

            if (verifyModule(*module, &outs())) {
                printf("Failed to verify module");
                exit(1);
            }

            auto module_error = llvm_jit->addModule(std::move(module));
            assert(!module_error && "Failed to add module\n");

            auto main_symbol = llvm_jit->lookup("main");
            assert(main_symbol && "Main function found");

            void (*main)() = (void (*)())(intptr_t)main_symbol->getAddress();
            main();
        }

        void convert_opcode() {
            u8 opcode = fetch_u8();

            switch (opcode) {
                case OP_ICONST_0:
                case OP_ICONST_1:
                case OP_ICONST_2:
                case OP_ICONST_3:
                case OP_ICONST_4:
                case OP_ICONST_5: {
                    push(make_int(opcode - 3));
                } break;
                case OP_BIPUSH: {
                    push(make_int(fetch_u8()));
                } break;
                case OP_SIPUSH: {
                    push(make_int(fetch_u16()));
                } break;
                case OP_ILOAD: {
                    load(fetch_u8());
                } break;
                case OP_ILOAD_0:
                case OP_ILOAD_1:
                case OP_ILOAD_2:
                case OP_ILOAD_3: {
                    load(opcode - 0x1a);
                } break;
                case OP_ISTORE: {
                    store(fetch_u8());
                } break;
                case OP_ISTORE_0:
                case OP_ISTORE_1:
                case OP_ISTORE_2:
                case OP_ISTORE_3: {
                    store(opcode - 0x3b);
                } break;
                case OP_POP: {
                    pop();
                } break;
                case OP_DUP: {
                    Value *val = pop();
                    push(val);
                    push(val);
                } break;
                case OP_IADD:
                case OP_ISUB:
                case OP_IMUL:
                case OP_IDIV:
                case OP_IREM:
                case OP_ISHL:
                case OP_ISHR: {
                    Value *l = pop();
                    Value *r = pop();
                    Instruction::BinaryOps op;

                    switch (opcode) {
                        case OP_IADD: op = Instruction::BinaryOps::Add; break;
                        case OP_ISUB: op = Instruction::BinaryOps::Sub; break;
                        case OP_IMUL: op = Instruction::BinaryOps::Mul; break;
                        case OP_IDIV: op = Instruction::BinaryOps::SDiv; break;
                        case OP_IREM: op = Instruction::BinaryOps::SRem; break;
                        case OP_ISHL: op = Instruction::BinaryOps::AShr; break;
                        case OP_ISHR: op = Instruction::BinaryOps::AShr; break;
                    }

                    push(irb->CreateBinOp(op, l, r));
                } break;
                case OP_INEG: {
                    Value *v = pop();
                    v = irb->CreateNeg(v);
                    push(v);
                } break;
                case OP_IINC: {
                    u8 index = fetch_u8();
                    Value *local = load_local(index);
                    Value *added = irb->CreateAdd(local, make_int(1));
                    store(index, added);
                } break;
                case OP_RETURN:
                    irb->CreateRetVoid();
                    break;
                case OP_IRETURN:
                    irb->CreateRet(pop());
                    break;
                case OP_INVOKESPECIAL: {
                    u16 method_index = fetch_u16();
                    CP_Info method_ref = get_cp_info(method_index);
                    CP_Info class_name = get_class_name(method_ref.class_index);
                    CP_Info member_name = get_member_name(method_ref.name_and_type_index);

                    Method *m = find_method(class_name.utf8, member_name.utf8);
                    if (m) {
                        call(m, true);
                    }
                } break;
                case OP_INVOKESTATIC: {
                    u16 method_index = fetch_u16();

                    CP_Info method_ref = get_cp_info(method_index);
                    CP_Info member_name = get_member_name(method_ref.name_and_type_index);

                    Method *m = find_method(member_name.utf8);
                    call(m, false);
                } break;
                case OP_NEW: {
                    u16 index = fetch_u16();

                    CP_Info constant_clazz = get_cp_info(index);
                    CP_Info class_name = get_cp_info(constant_clazz.name_index);

                    // push(make_object(class_name.utf8));
                } break;
            }
        }

        void call(Method *m, bool on_object) {
            if (!m->llvm_ref) {
                convert_method(m);
            }

            Array<Value *> args;
            for (u16 i = 0; i < m->type->parameters.length; ++i) {
                args.add(pop());
            }

            if (on_object) {
                sp--;
            }

            Value *ret_val = irb->CreateCall(m->llvm_ref, ArrayRef(args.data, args.length));

            if (m->type->return_type->type == NType::INT) {
                push(ret_val);
            }
        }

        Function *convert_method(Method *m) {
            Function *fn = convert_function_header(m);
            m->llvm_ref = fn;
            Code ci = find_code(m);
            method = m;

            function_setup(ci);

            u16 i = 0;
            for (auto &arg : fn->args()) {
                if (m->type->parameters[i]->type == NType::INT) {
                    store(i, &arg);
                }
                ++i;
            }

            while (ip < ci.code + ci.code_length) {
                convert_opcode();
            }

            return fn;
        }

        Function *convert_function_header(Method *m) {
            Type *ret_type = convert_type(m->type->return_type);

            Array<Type *> params;
            for (auto pty: m->type->parameters) {
                if (pty->type == NType::INT) {
                    params.add(convert_type(pty));
                }
            }

            String fn_name = m->name;
            if (fn_name != "main") {
                fn_name = clazz->name + to_string(".") + fn_name;
            }

            auto fty = FunctionType::get(ret_type, ArrayRef(params.data, params.length), false);
            auto fn = Function::Create(fty, Function::ExternalLinkage, STR_REF(fn_name), *module);

            BasicBlock *bb = BasicBlock::Create(context, "", fn);
            irb->SetInsertPoint(bb);

            return fn;
        }

        void function_setup(Code ci) {
            stack = (Value **) malloc(ci.max_stack * sizeof(Value *));
            locals = (Value **) malloc(ci.max_locals * sizeof(Value *));
            ip = ci.code;
            sp = 0;

            for (u16 i = 0; i < ci.max_stack; ++i) {
                AllocaInst *a = irb->CreateAlloca(llty_i32, 0, "s" + std::to_string(i));
                stack[i] = a;
            }

            for (u16 i = 0; i < ci.max_locals; ++i) {
                AllocaInst *a = irb->CreateAlloca(llty_i32, 0, "l" + std::to_string(i));
                locals[i] = a;
            }
        }

        Type *convert_type(NType *type) {
            switch (type->type) {
                case NType::ARRAY:
                    return convert_type(type->element_type)->getPointerTo();
                case NType::CLASS: {
                    if (type->clazz_name == "java/lang/String") {
                        return llty_i8->getPointerTo();
                    }
                    assert(0 && "Class not implemented");
                    return 0;
                }
                case NType::INT:
                    return llty_i32;
                case NType::VOID:
                    return llty_void;
            }

            assert(0 && "Type conversion not implemented");
            return 0;
        }

        void push(Value *val) {
            irb->CreateStore(val, stack[sp++]);
        }

        Value *pop() {
            return irb->CreateLoad(stack[--sp]);
        }

        void store(u8 index, Value *value) {
            irb->CreateStore(value, locals[index]);
        }

        void store(u8 index) {
            store(index, pop());
        }

        void load(u8 index) {
            push(load_local(index));
        }

        Value *load_local(u8 index) {
            return irb->CreateLoad(locals[index]);
        }

        void optimize() {
            legacy::PassManager *pm = new legacy::PassManager();
            PassManagerBuilder pmb;
            pmb.OptLevel = 2;
            pmb.SizeLevel = 0;
            pmb.DisableUnrollLoops = false;
            pmb.LoopVectorize = true;
            pmb.SLPVectorize = true;
            pmb.populateModulePassManager(*pm);
            pm->run(*module);
        }

        Value *make_int(s32 v) {
            return ConstantInt::get(llty_i32, v);
        }
    };
}