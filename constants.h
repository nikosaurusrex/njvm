enum {
	CONSTANT_Utf8 = 1,
	CONSTANT_Integer = 3,
	CONSTANT_Float = 4,
	CONSTANT_Long = 5,
	CONSTANT_Double = 6,
	CONSTANT_Class = 7,
	CONSTANT_String = 8,
	CONSTANT_Fieldref = 9,
	CONSTANT_Methodref = 10,
	CONSTANT_InterfaceMethodref = 11,
	CONSTANT_NameAndType = 12,
	CONSTANT_MethodHandle = 15,
	CONSTANT_MethodType = 16,
	CONSTANT_Dynamic = 17,
	CONSTANT_InvokeDynamic = 18,
	CONSTANT_Module = 19,
	CONSTANT_Package = 20
};

enum {
	ACC_PUBLIC = 0x0001,
	ACC_PRIVATE = 0x0002,
	ACC_PROTECTED = 0x0004,
	ACC_STATIC = 0x0008,
	ACC_FINAL = 0x0010,
	ACC_SYNCHRONIZED = 0x0020,
	ACC_SUPER = 0x0020,
	ACC_VOLATILE = 0x0040,
	ACC_BRIDGE = 0x0040,
	ACC_TRANSIENT = 0x0080,
	ACC_VARARGS = 0x0080,
	ACC_NATIVE = 0x1000,
	ACC_SYNTHETIC = 0x1000,
	ACC_ANNOTATION = 0x2000,
	ACC_ENUM = 0x4000,
	ACC_ABSTRACT = 0x4000,
	ACC_STRICT = 0x8000,
	ACC_MODULE = 0x8000,
};

enum {
	OP_ICONST_0 = 0x3,
	OP_ICONST_1 = 0x4,
	OP_ICONST_2 = 0x5,
	OP_ICONST_3 = 0x6,
	OP_ICONST_4 = 0x7,
	OP_ICONST_5 = 0x8,
	OP_BIPUSH = 0x10,
	OP_SIPUSH = 0x11,
	OP_LDC = 0x12,
	OP_ILOAD = 0x15,
	OP_ALOAD = 0x19,
	OP_ILOAD_0 = 0x1a,
	OP_ILOAD_1 = 0x1b,
	OP_ILOAD_2 = 0x1c,
	OP_ILOAD_3 = 0x1d,
	OP_ALOAD_0 = 0x2a,
	OP_ALOAD_1 = 0x2b,
	OP_ALOAD_2 = 0x2c,
	OP_ALOAD_3 = 0x2d,
    OP_IALOAD = 0x2e,
    OP_LALOAD = 0x2f,
    OP_BALOAD = 0x33,
    OP_SALOAD = 0x35,
	OP_ASTORE = 0x3a,
	OP_ASTORE_0 = 0x4b,
	OP_ASTORE_1 = 0x4c,
	OP_ASTORE_2 = 0x4d,
	OP_ASTORE_3 = 0x4e,
	OP_ISTORE = 0x36,
	OP_ISTORE_0 = 0x3b,
	OP_ISTORE_1 = 0x3c,
	OP_ISTORE_2 = 0x3d,
	OP_ISTORE_3 = 0x3e,
    OP_IASTORE = 0x4f,
    OP_LASTORE = 0x50,
    OP_BASTORE = 0x54,
    OP_SASTORE = 0x56,
	OP_POP = 0x57,
	OP_DUP = 0x59,
	OP_IADD = 0x60,
	OP_ISUB = 0x64,
	OP_IMUL = 0x68,
	OP_IDIV = 0x6c,
	OP_IREM = 0x70,
	OP_INEG = 0x74,
	OP_ISHL = 0x78,
	OP_ISHR = 0x7a,
	OP_IAND = 0x7e,
	OP_IOR = 0x80,
	OP_IINC = 0x84,
    OP_IFEQ = 0x99,
    OP_IFNE = 0x9a,
    OP_IFLT = 0x9b,
    OP_IFGE = 0x9c,
    OP_IFGT = 0x9d,
    OP_IFLE = 0x9e,
    OP_IF_ICMPEQ = 0x9f,
    OP_IF_ICMPNE = 0xa0,
    OP_IF_ICMPLT = 0xa1,
    OP_IF_ICMPGE = 0xa2,
    OP_IF_ICMPGT = 0xa3,
    OP_IF_ICMPLE = 0xa4,
	OP_GOTO = 0xa7,
	OP_IRETURN = 0xac,
	OP_RETURN = 0xb1,
	OP_GETSTATIC = 0xb2,
    OP_PUTSTATIC = 0xb3,
	OP_INVOKEVIRTUAL = 0xb6,
	OP_INVOKESPECIAL = 0xb7,
	OP_INVOKESTATIC = 0xb8,
	OP_NEW = 0xbb,
    OP_NEWARRAY = 0xbc,
    OP_ARRAYLENGTH = 0xbe
};

enum {
    INST_LABEL = 9,
    INST_LABELW = 10,
};

enum {
    TYPE_BOOLEAN = 4,
    TYPE_CHAR = 5,
    TYPE_FLOAT = 6,
    TYPE_DOUBLE = 7,
    TYPE_BYTE = 8,
    TYPE_SHORT = 9,
    TYPE_INT = 10,
    TYPE_LONG = 11,
};