/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Access .dex (Dalvik Executable Format) files.  The code here assumes that
 * the DEX file has been rewritten (byte-swapped, word-aligned) and that
 * the contents can be directly accessed as a collection of C arrays.  Please
 * see docs/dalvik/dex-format.html for a detailed description.
 *
 * The structure and field names were chosen to match those in the DEX spec.
 *
 * It's generally assumed that the DEX file will be stored in shared memory,
 * obviating the need to copy code and constant pool entries into newly
 * allocated storage.  Maintaining local pointers to items in the shared area
 * is valid and encouraged.
 *
 * All memory-mapped structures are 32-bit aligned unless otherwise noted.
 */

#ifndef LIBDEX_DEXFILE_H_
#define LIBDEX_DEXFILE_H_

#ifndef LOG_TAG
# define LOG_TAG "libdex"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

/*
 * If "very verbose" logging is enabled, make it equivalent to ALOGV.
 * Otherwise, make it disappear.
 *
 * Define this above the #include "Dalvik.h" to enable for only a
 * single file.
 */
/* #define VERY_VERBOSE_LOG */
#if defined(VERY_VERBOSE_LOG)
# define LOGVV      ALOGV
# define IF_LOGVV() IF_ALOGV()
#else
# define LOGVV(...) ((void)0)
# define IF_LOGVV() if (false)
#endif

/*
 * These match the definitions in the VM specification.
 */
typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;


/*
 * gcc-style inline management -- ensures we have a copy of all functions
 * in the library, so code that links against us will work whether or not
 * it was built with optimizations enabled.
 */
#ifndef _DEX_GEN_INLINES             /* only defined by DexInlines.c */
# define DEX_INLINE extern inline
#else
# define DEX_INLINE
#endif

/* DEX file magic number */
#define DEX_MAGIC       "dex\n"

/* The version for android N, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * use default methods.
 */
#define DEX_MAGIC_VERS_37  "037\0"

/* The version for android O, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * contain invoke-custom, invoke-polymorphic, call-sites, and method handles.
 */
#define DEX_MAGIC_VERS_38  "038\0"

/* The version for android P, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * contain const-method-handle and const-proto.
 */
#define DEX_MAGIC_VERS_39  "039\0"

/* current version, encoded in 4 bytes of ASCII */
#define DEX_MAGIC_VERS  "036\0"

/*
 * older but still-recognized version (corresponding to Android API
 * levels 13 and earlier
 */
#define DEX_MAGIC_VERS_API_13  "035\0"

/* same, but for optimized DEX header */
#define DEX_OPT_MAGIC   "dey\n"
#define DEX_OPT_MAGIC_VERS  "036\0"

#define DEX_DEP_MAGIC   "deps"

/*
 * 160-bit SHA-1 digest.
 */
enum { kSHA1DigestLen = 20,
       kSHA1DigestOutputLen = kSHA1DigestLen*2 +1 };

/* general constants */
enum {
    kDexEndianConstant = 0x12345678,    /* the endianness indicator */
    kDexNoIndex = 0xffffffff,           /* not a valid index value */
};

/*
 * Enumeration of all the primitive types.
 */
enum PrimitiveType {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
};

/*
 * access flags and masks; the "standard" ones are all <= 0x4000
 *
 * Note: There are related declarations in vm/oo/Object.h in the ClassFlags
 * enum.
 */
enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
                       0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
        (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
                | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
        (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
                | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
                | ACC_DECLARED_SYNCHRONIZED),
};

/* annotation constants */
enum {
    kDexVisibilityBuild         = 0x00,     /* annotation visibility */
    kDexVisibilityRuntime       = 0x01,
    kDexVisibilitySystem        = 0x02,

    kDexAnnotationByte          = 0x00,
    kDexAnnotationShort         = 0x02,
    kDexAnnotationChar          = 0x03,
    kDexAnnotationInt           = 0x04,
    kDexAnnotationLong          = 0x06,
    kDexAnnotationFloat         = 0x10,
    kDexAnnotationDouble        = 0x11,
    kDexAnnotationMethodType    = 0x15,
    kDexAnnotationMethodHandle  = 0x16,
    kDexAnnotationString        = 0x17,
    kDexAnnotationType          = 0x18,
    kDexAnnotationField         = 0x19,
    kDexAnnotationMethod        = 0x1a,
    kDexAnnotationEnum          = 0x1b,
    kDexAnnotationArray         = 0x1c,
    kDexAnnotationAnnotation    = 0x1d,
    kDexAnnotationNull          = 0x1e,
    kDexAnnotationBoolean       = 0x1f,

    kDexAnnotationValueTypeMask = 0x1f,     /* low 5 bits */
    kDexAnnotationValueArgShift = 5,
};

/* map item type codes */
enum {
    kDexTypeHeaderItem               = 0x0000,
    kDexTypeStringIdItem             = 0x0001,
    kDexTypeTypeIdItem               = 0x0002,
    kDexTypeProtoIdItem              = 0x0003,
    kDexTypeFieldIdItem              = 0x0004,
    kDexTypeMethodIdItem             = 0x0005,
    kDexTypeClassDefItem             = 0x0006,
    kDexTypeCallSiteIdItem           = 0x0007,
    kDexTypeMethodHandleItem         = 0x0008,
    kDexTypeMapList                  = 0x1000,
    kDexTypeTypeList                 = 0x1001,
    kDexTypeAnnotationSetRefList     = 0x1002,
    kDexTypeAnnotationSetItem        = 0x1003,
    kDexTypeClassDataItem            = 0x2000,
    kDexTypeCodeItem                 = 0x2001,
    kDexTypeStringDataItem           = 0x2002,
    kDexTypeDebugInfoItem            = 0x2003,
    kDexTypeAnnotationItem           = 0x2004,
    kDexTypeEncodedArrayItem         = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};

/* auxillary data section chunk codes */
enum {
    kDexChunkClassLookup            = 0x434c4b50,   /* CLKP */
    kDexChunkRegisterMaps           = 0x524d4150,   /* RMAP */

    kDexChunkEnd                    = 0x41454e44,   /* AEND */
};

/* debug info opcodes and constants */
enum {
    DBG_END_SEQUENCE         = 0x00,
    DBG_ADVANCE_PC           = 0x01,
    DBG_ADVANCE_LINE         = 0x02,
    DBG_START_LOCAL          = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL            = 0x05,
    DBG_RESTART_LOCAL        = 0x06,
    DBG_SET_PROLOGUE_END     = 0x07,
    DBG_SET_EPILOGUE_BEGIN   = 0x08,
    DBG_SET_FILE             = 0x09,
    DBG_FIRST_SPECIAL        = 0x0a,
    DBG_LINE_BASE            = -4,
    DBG_LINE_RANGE           = 15,
};

/*
 * Direct-mapped "header_item" struct.
 */
struct DexHeader {
    u1  magic[8];       //取值必须是字符串 "dex\n035\0" 或者字节byte数组 {0x64 0x65 0x78 0x0a 0x30 0x33 0x35 0x00}
    u4  checksum;       //文件内容的校验和,不包括magic和自己,主要用于检查文件是否损坏
    u1  signature[kSHA1DigestLen];      //签名信息,不包括 magic\checksum和自己
    u4  fileSize;       //整个文件的长度,单位为字节,包括所有的内容
    u4  headerSize;     //默认是0x70个字节
    u4  endianTag;      //大小端标签，标准.dex文件为小端，此项一般固定为0x12345678常量
    u4  linkSize;       //链接数据的大小
    u4  linkOff;        //链接数据的偏移值
    u4  mapOff;         //map item的偏移地址，该item属于data区里的内容，值要大于等于dataOff的大小
    u4  stringIdsSize;      //DEX中用到的所有字符串内容的大小*
    u4  stringIdsOff;       //DEX中用到的所有字符串内容的偏移量
    u4  typeIdsSize;        //DEX中类型数据结构的大小
    u4  typeIdsOff;         //DEX中类型数据结构的偏移值
    u4  protoIdsSize;       //DEX中的元数据信息数据结构的大小
    u4  protoIdsOff;        //DEX中的元数据信息数据结构的偏移值
    u4  fieldIdsSize;       //DEX中字段信息数据结构的大小
    u4  fieldIdsOff;        //DEX中字段信息数据结构的偏移值
    u4  methodIdsSize;      //DEX中方法信息数据结构的大小
    u4  methodIdsOff;       //DEX中方法信息数据结构的偏移值
    u4  classDefsSize;      //DEX中的类信息数据结构的大小
    u4  classDefsOff;       //DEX中的类信息数据结构的偏移值
    u4  dataSize;           //DEX中数据区域的结构信息的大小
    u4  dataOff;            //DEX中数据区域的结构信息的偏移值
};

/*
 * Direct-mapped "map_item".
 */
struct DexMapItem {
    u2 type;              /* type code (see kDexType* above) */
    u2 unused;
    u4 size;              /* count of items of the indicated type */
    u4 offset;            /* file offset to the start of data */
};

/*
 * Direct-mapped "map_list".
 */
struct DexMapList {
    u4  size;               /* #of entries in list */
    DexMapItem list[1];     /* entries */
};

/*
 * Direct-mapped "string_id_item".
 */
struct DexStringId {
    //用于指明 string_data_item 位置文件的位置
    u4 stringDataOff;      /* file offset to string_data_item */
};

/*
 * Direct-mapped "type_id_item".
 */
struct DexTypeId {
    u4  descriptorIdx;      /* 指向string_ids的索引 */
};

/*
 * Direct-mapped "field_id_item".
 */
struct DexFieldId {
    u2  classIdx;           /* field所属的class类型，class_idx的值时type_ids的一个index，指向所属的类 */
    u2  typeIdx;            /* field的类型，值是type_ids的一个index */
    u4  nameIdx;            /* field的名称，它的值是string_ids的一个index */
};

/*
 * Direct-mapped "method_id_item".
 */
struct DexMethodId {
    u2  classIdx;           /* method所属的class类型，class_idx的值是type_ids的一个index，必须指向一个class类型 */
    u2  protoIdx;           /* method的原型，指向proto_ids的一个index */
    u4  nameIdx;            /* method的名称，值为string_ids的一个index */
};

/*
 * Direct-mapped "proto_id_item".
 */
struct DexProtoId {
    u4  shortyIdx;          /* 值为一个string_ids的index号，用来说明该method原型 */
    u4  returnTypeIdx;      /* 值为一个type_ids的index，表示该method原型的返回值类型 */
    u4  parametersOff;      /* 指定method原型的参数列表type_list，若method没有参数，则值为0. 参数的格式是type_list */
};

/*
 * Direct-mapped "class_def_item".
 */
struct DexClassDef {
    u4  classIdx;           /* 描述具体的class类型，值是type_ids的一个index，值必须是一个class类型，不能是数组或者基本类型 */
    u4  accessFlags;        /* 描述class的访问类型，如public,final,static等 */
    u4  superclassIdx;      /* 描述父类的类型，值必须是一个class类型，不能是数组雷兴国或者基本类型 */
    u4  interfacesOff;      /* 值为偏移地址，被指向的数据结构为type_list，class若没有interfaces，值为0 */
    u4  sourceFileIdx;      /* 表示源代码文件的信息，值为string_ids的一个index。若此项信息丢失，此项赋值为NO_INDEX=0xFFFFFFFF */
    u4  annotationsOff;     /* 值为偏移地址，指向的内容是该class的注解，位置在data区，格式为annotations_directory_item，若没有此项，值为0 */
    u4  classDataOff;       /* 值为偏移地址，指向的内容是该class的使用到的数据，位置在data区，格式为class_data_item。无偶没有此项，则值为0 */
    u4  staticValuesOff;    /* 值为偏移地址，指向data区里的一个列表，格式为encoded_array_item。若没有此项，值为0. */
};

/*
 * Direct-mapped "call_site_id_item"
 */
struct DexCallSiteId {
    u4  callSiteOff;        /* file offset to DexEncodedArray */
};

/*
 * Enumeration of method handle type codes.
 */
enum MethodHandleType {
    STATIC_PUT = 0x00,
    STATIC_GET = 0x01,
    INSTANCE_PUT = 0x02,
    INSTANCE_GET = 0x03,
    INVOKE_STATIC = 0x04,
    INVOKE_INSTANCE = 0x05,
    INVOKE_CONSTRUCTOR = 0x06,
    INVOKE_DIRECT = 0x07,
    INVOKE_INTERFACE = 0x08
};

/*
 * Direct-mapped "method_handle_item"
 */
struct DexMethodHandleItem {
    u2 methodHandleType;    /* type of method handle */
    u2 reserved1;           /* reserved for future use */
    u2 fieldOrMethodIdx;    /* index of associated field or method */
    u2 reserved2;           /* reserved for future use */
};

/*
 * Direct-mapped "type_item".
 */
struct DexTypeItem {
    u2  typeIdx;            /* index into typeIds */
};

/*
 * Direct-mapped "type_list".
 */
struct DexTypeList {
    u4  size;               /* #of entries in list */
    DexTypeItem list[1];    /* entries */
};

/*
 * 代码条目
 *
 *  (1) 一个 .dex 文件被分成了 9 个区 ，其中有一个索引区叫做class_defs ， 索引了 .dex 里面用到的 class ，以及对这个 class 的描述 。
 *  (2) class_defs 区 ， 里面其实是class_def_item 结构 。这个结构里描述了 LHello; 的各种信息 ，诸如名称 ，superclass , access flag，
 *       interface 等 。class_def_item 里有一个元素 class_data_off , 指向data 区里的一个 class_data_item 结构 ，
 *       用来描述 class 使用到的各种数据 。自此以后的结构都归于 data区了 。
 *  (3) class_data_item 结构 ，里描述值着 class 里使用到的 static field , instance field , direct_method ，和 virtual_method
 *      的数目和描述 。例子 Hello.dex 里 ，只有 2 个 direct_method , 其余的 field 和method 的数目都为 0 。描述 direct_method 的结构叫
 *      做 encoded_method ，是用来详细描述某个 method的 。
 *  (4) encoded_method 结构 ，描述某个 method 的 method 类型 ， access flags 和一个指向 code_item的偏移地址 ，里面存放的是该 method
 *      的具体实现 。
 *  (5) code_item ，结构里描述着某个 method 的具体实现 。
 *
 */
struct DexCode {
    u2  registersSize;  /* 本段代码使用到的寄存器数目 */
    u2  insSize;        /* method传入参数的数目 */
    u2  outsSize;       /* 本段代码调用其他方法时需要的参数个数 */
    u2  triesSize;      /* try_item结构的个数 */
    u4  debugInfoOff;       /* 偏移地址，指向本段代码的debug信息存放位置，是一个debug_info_item结构 */
    u4  insnsSize;          /* 指令列表的大小，以16-bit为单位。insns是instructions的缩写 */
    u2  insns[1];           /*  insns数组 */
    /* followed by optional u2 padding */
    /* followed by try_item[triesSize] */
    /* followed by uleb128 handlersSize */
    /* followed by catch_handler_item[handlersSize] */
};

/*
 * Direct-mapped "try_item".
 */
struct DexTry {
    u4  startAddr;          /* start address, in 16-bit code units */
    u2  insnCount;          /* instruction count, in 16-bit code units */
    u2  handlerOff;         /* offset in encoded handler data to handlers */
};

/*
 * Link table.  Currently undefined.
 */
struct DexLink {
    u1  bleargh;
};


/*
 * Direct-mapped "annotations_directory_item".
 */
struct DexAnnotationsDirectoryItem {
    u4  classAnnotationsOff;  /* offset to DexAnnotationSetItem */
    u4  fieldsSize;           /* count of DexFieldAnnotationsItem */
    u4  methodsSize;          /* count of DexMethodAnnotationsItem */
    u4  parametersSize;       /* count of DexParameterAnnotationsItem */
    /* followed by DexFieldAnnotationsItem[fieldsSize] */
    /* followed by DexMethodAnnotationsItem[methodsSize] */
    /* followed by DexParameterAnnotationsItem[parametersSize] */
};

/*
 * Direct-mapped "field_annotations_item".
 */
struct DexFieldAnnotationsItem {
    u4  fieldIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "method_annotations_item".
 */
struct DexMethodAnnotationsItem {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "parameter_annotations_item".
 */
struct DexParameterAnnotationsItem {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnotationSetRefList */
};

/*
 * Direct-mapped "annotation_set_ref_item".
 */
struct DexAnnotationSetRefItem {
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "annotation_set_ref_list".
 */
struct DexAnnotationSetRefList {
    u4  size;
    DexAnnotationSetRefItem list[1];
};

/*
 * Direct-mapped "annotation_set_item".
 */
struct DexAnnotationSetItem {
    u4  size;
    u4  entries[1];                 /* offset to DexAnnotationItem */
};

/*
 * Direct-mapped "annotation_item".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexAnnotationItem {
    u1  visibility;
    u1  annotation[1];              /* data in encoded_annotation format */
};

/*
 * Direct-mapped "encoded_array".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexEncodedArray {
    u1  array[1];                   /* data in encoded_array format */
};

/*
 * Lookup table for classes.  It provides a mapping from class name to
 * class definition.  Used by dexFindClass().
 *
 * We calculate this at DEX optimization time and embed it in the file so we
 * don't need the same hash table in every VM.  This is slightly slower than
 * a hash table with direct pointers to the items, but because it's shared
 * there's less of a penalty for using a fairly sparse table.
 */
struct DexClassLookup {
    int     size;                       // total size, including "size"
    int     numEntries;                 // size of table[]; always power of 2
    struct {
        u4      classDescriptorHash;    // class descriptor hash code
        int     classDescriptorOffset;  // in bytes, from start of DEX
        int     classDefOffset;         // in bytes, from start of DEX
    } table[1];
};

/*
 * Header added by DEX optimization pass.  Values are always written in
 * local byte and structure padding.  The first field (magic + version)
 * is guaranteed to be present and directly readable for all expected
 * compiler configurations; the rest is version-dependent.
 *
 * Try to keep this simple and fixed-size.
 */
struct DexOptHeader {
    u1  magic[8];           /* includes version number */

    u4  dexOffset;          /* file offset of DEX header */
    u4  dexLength;
    u4  depsOffset;         /* offset of optimized DEX dependency table */
    u4  depsLength;
    u4  optOffset;          /* file offset of optimized data tables */
    u4  optLength;

    u4  flags;              /* some info flags */
    u4  checksum;           /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};

#define DEX_OPT_FLAG_BIG            (1<<1)  /* swapped to big-endian */

#define DEX_INTERFACE_CACHE_SIZE    128     /* must be power of 2 */

/*
 * Structure representing a DEX file.
 *
 * Code should regard DexFile as opaque, using the API calls provided here
 * to access specific structures.
 */
struct DexFile {
    /* directly-mapped "opt" header */
    //const DexOptHeader* pOptHeader;

    /*
        对应关系如下
        DexHeader*    pHeader   ---->struct header_item dex_header
        DexStringId*  pStringIds---->struct string_id_list dex_string_ids
        DexTypeId*    pTypeIds  ---->struct type_id_list dex_type_ids
        DexFieldId*   pFieldIds ---->struct field_id_list dex_field_ids
        DexMethodId*  pMethodIds---->struct method_id_list dex_method_ids
        DexProtoId*   pProtoIds ---->struct proto_id_list dex_proto_ids
        DexClassDef*  pClassDefs---->struct class_def_item_list dex_class_defs
        DexLink*      pLinkData ---->struct map_list_type dex_map_list
     */

    /* pointers to directly-mapped structs and arrays in base DEX */
     DexHeader*    pHeader;        //DEX 文件头，记录了一些当前文件的信息以及其他数据结构在文件中的偏移量
     DexStringId*  pStringIds;     //数组,元素类型为string_id_item,存储字符串相关的信息
     DexTypeId*    pTypeIds;       //数组,存储类型相关的信息
     DexFieldId*   pFieldIds;      //数组,存储成员变量信息,包括变量名和类型等
     DexMethodId*  pMethodIds;     //数组,存储成员函数信息包括函数名 参数和返回值类型
     DexProtoId*   pProtoIds;      //数组,函数原型数据索引，记录了方法声明的字符串，返回类型和参数列表
     DexClassDef*  pClassDefs;     //数组,存储类的信息
     DexLink*      pLinkData;      //Dex文件重要的数据内容都存在data区域内,一些数据结构会通过如 xx_off这样的成员变量只想文件的某个位置,从该位置开始,存储了对应的数据结构的内容,而xx_off的位置一般落在data区域内

    /*
     * These are mapped out of the "auxillary" section, and may not be
     * included in the file.
     */
    const DexClassLookup* pClassLookup;
    const void*         pRegisterMapPool;       // RegisterMapClassPool

    /* points to start of DEX file data */
    const u1*           baseAddr;

    /* track memory overhead for auxillary structures */
    int                 overhead;

    /* additional app-specific data structures associated with the DEX */
    //void*               auxData;
};

/*
 * Utility function -- rounds up to the nearest power of 2.
 */
u4 dexRoundUpPower2(u4 val);

/*
 * Parse an optimized or unoptimized .dex file sitting in memory.
 *
 * On success, return a newly-allocated DexFile.
 */
DexFile* dexFileParse(const u1* data, size_t length, int flags);

/* bit values for "flags" argument to dexFileParse */
enum {
    kDexParseDefault            = 0,
    kDexParseVerifyChecksum     = 1,
    kDexParseContinueOnError    = (1 << 1),
};

/*
 * Fix the byte ordering of all fields in the DEX file, and do
 * structural verification. This is only required for code that opens
 * "raw" DEX files, such as the DEX optimizer.
 *
 * Return 0 on success.
 */
int dexSwapAndVerify(u1* addr, int len);

/*
 * Detect the file type of the given memory buffer via magic number.
 * Call dexSwapAndVerify() on an unoptimized DEX file, do nothing
 * but return successfully on an optimized DEX file, and report an
 * error for all other cases.
 *
 * Return 0 on success.
 */
int dexSwapAndVerifyIfNecessary(u1* addr, size_t len);

/*
 * Check to see if the file magic and format version in the given
 * header are recognized as valid. Returns true if they are
 * acceptable.
 */
bool dexHasValidMagic(const DexHeader* pHeader);

/*
 * Compute DEX checksum.
 */
u4 dexComputeChecksum(const DexHeader* pHeader);

/*
 * Free a DexFile structure, along with any associated structures.
 */
void dexFileFree(DexFile* pDexFile);

/*
 * Create class lookup table.
 */
DexClassLookup* dexCreateClassLookup(DexFile* pDexFile);

/*
 * Find a class definition by descriptor.
 */
const DexClassDef* dexFindClass(const DexFile* pFile, const char* descriptor);

/*
 * Set up the basic raw data pointers of a DexFile. This function isn't
 * meant for general use.
 */
void dexFileSetupBasicPointers(DexFile* pDexFile, const u1* data);

/* return the DexMapList of the file, if any */
DEX_INLINE const DexMapList* dexGetMap(const DexFile* pDexFile) {
    u4 mapOff = pDexFile->pHeader->mapOff;

    if (mapOff == 0) {
        return NULL;
    } else {
        return (const DexMapList*) (pDexFile->baseAddr + mapOff);
    }
}

/* return the const char* string data referred to by the given string_id */
DEX_INLINE const char* dexGetStringData(const DexFile* pDexFile,
        const DexStringId* pStringId) {
    const u1* ptr = pDexFile->baseAddr + pStringId->stringDataOff;

    // Skip the uleb128 length.
    while (*(ptr++) > 0x7f) /* empty */ ;

    return (const char*) ptr;
}
/* return the StringId with the specified index */
DEX_INLINE const DexStringId* dexGetStringId(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->stringIdsSize);
    return &pDexFile->pStringIds[idx];
}
/* return the UTF-8 encoded string with the specified string_id index */
DEX_INLINE const char* dexStringById(const DexFile* pDexFile, u4 idx) {
    const DexStringId* pStringId = dexGetStringId(pDexFile, idx);
    return dexGetStringData(pDexFile, pStringId);
}

/* Return the UTF-8 encoded string with the specified string_id index,
 * also filling in the UTF-16 size (number of 16-bit code points).*/
const char* dexStringAndSizeById(const DexFile* pDexFile, u4 idx,
        u4* utf16Size);

/* return the TypeId with the specified index */
DEX_INLINE const DexTypeId* dexGetTypeId(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->typeIdsSize);
    return &pDexFile->pTypeIds[idx];
}

/*
 * Get the descriptor string associated with a given type index.
 * The caller should not free() the returned string.
 */
DEX_INLINE const char* dexStringByTypeIdx(const DexFile* pDexFile, u4 idx) {
    const DexTypeId* typeId = dexGetTypeId(pDexFile, idx);
    return dexStringById(pDexFile, typeId->descriptorIdx);
}

/* return the MethodId with the specified index */
DEX_INLINE const DexMethodId* dexGetMethodId(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->methodIdsSize);
    return &pDexFile->pMethodIds[idx];
}

/* return the FieldId with the specified index */
DEX_INLINE const DexFieldId* dexGetFieldId(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->fieldIdsSize);
    return &pDexFile->pFieldIds[idx];
}

/* return the ProtoId with the specified index */
DEX_INLINE const DexProtoId* dexGetProtoId(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->protoIdsSize);
    return &pDexFile->pProtoIds[idx];
}

/*
 * Get the parameter list from a ProtoId. The returns NULL if the ProtoId
 * does not have a parameter list.
 */
DEX_INLINE const DexTypeList* dexGetProtoParameters(
    const DexFile *pDexFile, const DexProtoId* pProtoId) {
    if (pProtoId->parametersOff == 0) {
        return NULL;
    }
    return (const DexTypeList*)
        (pDexFile->baseAddr + pProtoId->parametersOff);
}

/* return the ClassDef with the specified index */
DEX_INLINE const DexClassDef* dexGetClassDef(const DexFile* pDexFile, u4 idx) {
    assert(idx < pDexFile->pHeader->classDefsSize);
    return &pDexFile->pClassDefs[idx];
}

/* given a ClassDef pointer, recover its index */
DEX_INLINE u4 dexGetIndexForClassDef(const DexFile* pDexFile,
    const DexClassDef* pClassDef)
{
    assert(pClassDef >= pDexFile->pClassDefs &&
           pClassDef < pDexFile->pClassDefs + pDexFile->pHeader->classDefsSize);
    return pClassDef - pDexFile->pClassDefs;
}

/* get the interface list for a DexClass */
DEX_INLINE const DexTypeList* dexGetInterfacesList(const DexFile* pDexFile,
    const DexClassDef* pClassDef)
{
    if (pClassDef->interfacesOff == 0)
        return NULL;
    return (const DexTypeList*)
        (pDexFile->baseAddr + pClassDef->interfacesOff);
}
/* return the Nth entry in a DexTypeList. */
DEX_INLINE const DexTypeItem* dexGetTypeItem(const DexTypeList* pList,
    u4 idx)
{
    assert(idx < pList->size);
    return &pList->list[idx];
}
/* return the type_idx for the Nth entry in a TypeList */
DEX_INLINE u4 dexTypeListGetIdx(const DexTypeList* pList, u4 idx) {
    const DexTypeItem* pItem = dexGetTypeItem(pList, idx);
    return pItem->typeIdx;
}

/* get the static values list for a DexClass */
DEX_INLINE const DexEncodedArray* dexGetStaticValuesList(
    const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->staticValuesOff == 0)
        return NULL;
    return (const DexEncodedArray*)
        (pDexFile->baseAddr + pClassDef->staticValuesOff);
}

/* get the annotations directory item for a DexClass */
DEX_INLINE const DexAnnotationsDirectoryItem* dexGetAnnotationsDirectoryItem(
    const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->annotationsOff == 0)
        return NULL;
    return (const DexAnnotationsDirectoryItem*)
        (pDexFile->baseAddr + pClassDef->annotationsOff);
}

/* get the source file string */
DEX_INLINE const char* dexGetSourceFile(
    const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->sourceFileIdx == 0xffffffff)
        return NULL;
    return dexStringById(pDexFile, pClassDef->sourceFileIdx);
}

/* get the size, in bytes, of a DexCode */
size_t dexGetDexCodeSize(const DexCode* pCode);

/* Get the list of "tries" for the given DexCode. */
DEX_INLINE const DexTry* dexGetTries(const DexCode* pCode) {
    const u2* insnsEnd = &pCode->insns[pCode->insnsSize];

    // Round to four bytes.
    if ((((uintptr_t) insnsEnd) & 3) != 0) {
        insnsEnd++;
    }

    return (const DexTry*) insnsEnd;
}

/* Get the base of the encoded data for the given DexCode. */
DEX_INLINE const u1* dexGetCatchHandlerData(const DexCode* pCode) {
    const DexTry* pTries = dexGetTries(pCode);
    return (const u1*) &pTries[pCode->triesSize];
}

/* get a pointer to the start of the debugging data */
DEX_INLINE const u1* dexGetDebugInfoStream(const DexFile* pDexFile,
    const DexCode* pCode)
{
    if (pCode->debugInfoOff == 0) {
        return NULL;
    } else {
        return pDexFile->baseAddr + pCode->debugInfoOff;
    }
}

/* DexClassDef convenience - get class descriptor */
DEX_INLINE const char* dexGetClassDescriptor(const DexFile* pDexFile,
    const DexClassDef* pClassDef)
{
    return dexStringByTypeIdx(pDexFile, pClassDef->classIdx);
}

/* DexClassDef convenience - get superclass descriptor */
DEX_INLINE const char* dexGetSuperClassDescriptor(const DexFile* pDexFile,
    const DexClassDef* pClassDef)
{
    if (pClassDef->superclassIdx == 0)
        return NULL;
    return dexStringByTypeIdx(pDexFile, pClassDef->superclassIdx);
}

/* DexClassDef convenience - get class_data_item pointer */
DEX_INLINE const u1* dexGetClassData(const DexFile* pDexFile,
    const DexClassDef* pClassDef)
{
    if (pClassDef->classDataOff == 0)
        return NULL;
    return (const u1*) (pDexFile->baseAddr + pClassDef->classDataOff);
}

/* Get an annotation set at a particular offset. */
DEX_INLINE const DexAnnotationSetItem* dexGetAnnotationSetItem(
    const DexFile* pDexFile, u4 offset)
{
    if (offset == 0) {
        return NULL;
    }
    return (const DexAnnotationSetItem*) (pDexFile->baseAddr + offset);
}
/* get the class' annotation set */
DEX_INLINE const DexAnnotationSetItem* dexGetClassAnnotationSet(
    const DexFile* pDexFile, const DexAnnotationsDirectoryItem* pAnnoDir)
{
    return dexGetAnnotationSetItem(pDexFile, pAnnoDir->classAnnotationsOff);
}

/* get the class' field annotation list */
DEX_INLINE const DexFieldAnnotationsItem* dexGetFieldAnnotations(
    const DexFile* pDexFile, const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    if (pAnnoDir->fieldsSize == 0)
        return NULL;

    // Skip past the header to the start of the field annotations.
    return (const DexFieldAnnotationsItem*) &pAnnoDir[1];
}

/* get field annotation list size */
DEX_INLINE int dexGetFieldAnnotationsSize(const DexFile* pDexFile,
    const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    return pAnnoDir->fieldsSize;
}

/* return a pointer to the field's annotation set */
DEX_INLINE const DexAnnotationSetItem* dexGetFieldAnnotationSetItem(
    const DexFile* pDexFile, const DexFieldAnnotationsItem* pItem)
{
    return dexGetAnnotationSetItem(pDexFile, pItem->annotationsOff);
}

/* get the class' method annotation list */
DEX_INLINE const DexMethodAnnotationsItem* dexGetMethodAnnotations(
    const DexFile* pDexFile, const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    if (pAnnoDir->methodsSize == 0)
        return NULL;

    /*
     * Skip past the header and field annotations to the start of the
     * method annotations.
     */
    const u1* addr = (const u1*) &pAnnoDir[1];
    addr += pAnnoDir->fieldsSize * sizeof (DexFieldAnnotationsItem);
    return (const DexMethodAnnotationsItem*) addr;
}

/* get method annotation list size */
DEX_INLINE int dexGetMethodAnnotationsSize(const DexFile* pDexFile,
    const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    return pAnnoDir->methodsSize;
}

/* return a pointer to the method's annotation set */
DEX_INLINE const DexAnnotationSetItem* dexGetMethodAnnotationSetItem(
    const DexFile* pDexFile, const DexMethodAnnotationsItem* pItem)
{
    return dexGetAnnotationSetItem(pDexFile, pItem->annotationsOff);
}

/* get the class' parameter annotation list */
DEX_INLINE const DexParameterAnnotationsItem* dexGetParameterAnnotations(
    const DexFile* pDexFile, const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    if (pAnnoDir->parametersSize == 0)
        return NULL;

    /*
     * Skip past the header, field annotations, and method annotations
     * to the start of the parameter annotations.
     */
    const u1* addr = (const u1*) &pAnnoDir[1];
    addr += pAnnoDir->fieldsSize * sizeof (DexFieldAnnotationsItem);
    addr += pAnnoDir->methodsSize * sizeof (DexMethodAnnotationsItem);
    return (const DexParameterAnnotationsItem*) addr;
}

/* get method annotation list size */
DEX_INLINE int dexGetParameterAnnotationsSize(const DexFile* pDexFile,
    const DexAnnotationsDirectoryItem* pAnnoDir)
{
    (void) pDexFile;
    return pAnnoDir->parametersSize;
}

/* return the parameter annotation ref list */
DEX_INLINE const DexAnnotationSetRefList* dexGetParameterAnnotationSetRefList(
    const DexFile* pDexFile, const DexParameterAnnotationsItem* pItem)
{
    if (pItem->annotationsOff == 0) {
        return NULL;
    }
    return (const DexAnnotationSetRefList*) (pDexFile->baseAddr + pItem->annotationsOff);
}

/* get method annotation list size */
DEX_INLINE int dexGetParameterAnnotationSetRefSize(const DexFile* pDexFile,
    const DexParameterAnnotationsItem* pItem)
{
    if (pItem->annotationsOff == 0) {
        return 0;
    }
    return dexGetParameterAnnotationSetRefList(pDexFile, pItem)->size;
}

/* return the Nth entry from an annotation set ref list */
DEX_INLINE const DexAnnotationSetRefItem* dexGetParameterAnnotationSetRef(
    const DexAnnotationSetRefList* pList, u4 idx)
{
    assert(idx < pList->size);
    return &pList->list[idx];
}

/* given a DexAnnotationSetRefItem, return the DexAnnotationSetItem */
DEX_INLINE const DexAnnotationSetItem* dexGetSetRefItemItem(
    const DexFile* pDexFile, const DexAnnotationSetRefItem* pItem)
{
    return dexGetAnnotationSetItem(pDexFile, pItem->annotationsOff);
}

/* return the Nth annotation offset from a DexAnnotationSetItem */
DEX_INLINE u4 dexGetAnnotationOff(
    const DexAnnotationSetItem* pAnnoSet, u4 idx)
{
    assert(idx < pAnnoSet->size);
    return pAnnoSet->entries[idx];
}

/* return the Nth annotation item from a DexAnnotationSetItem */
DEX_INLINE const DexAnnotationItem* dexGetAnnotationItem(
    const DexFile* pDexFile, const DexAnnotationSetItem* pAnnoSet, u4 idx)
{
    u4 offset = dexGetAnnotationOff(pAnnoSet, idx);
    if (offset == 0) {
        return NULL;
    }
    return (const DexAnnotationItem*) (pDexFile->baseAddr + offset);
}

/*
 * Get the type descriptor character associated with a given primitive
 * type. This returns '\0' if the type is invalid.
 */
char dexGetPrimitiveTypeDescriptorChar(PrimitiveType type);

/*
 * Get the type descriptor string associated with a given primitive
 * type.
 */
const char* dexGetPrimitiveTypeDescriptor(PrimitiveType type);

/*
 * Get the boxed type descriptor string associated with a given
 * primitive type. This returns NULL for an invalid type, including
 * particularly for type "void". In the latter case, even though there
 * is a class Void, there's no such thing as a boxed instance of it.
 */
const char* dexGetBoxedTypeDescriptor(PrimitiveType type);

/*
 * Get the primitive type constant from the given descriptor character.
 * This returns PRIM_NOT (note: this is a 0) if the character is invalid
 * as a primitive type descriptor.
 */
PrimitiveType dexGetPrimitiveTypeFromDescriptorChar(char descriptorChar);

#endif  // LIBDEX_DEXFILE_H_
