#include <iostream>
#include <fstream>
#include "DexFile.h"
#include <stdio.h>
#include <string.h>

using namespace std;


#define N 80

/**
 * @brief main
 *  将字符行内单字之间的空白符平均分配插入到单字之间，
 *  以实现字符行排版。
 *
 *  首先要统计字符行内单字个数，字符行内的空白字符数。然后计算单字之间
 *  应该平均分配的空白字符数，另外约定多余的空白字符插在前面的单字间隔中
 *  前面的每个间隔多一个空白符，插完为止。
 * @return
 */

/**
 * 读取文件到内存中
 * @param file_Path
 * @return 返回内存首地址
 */
char* Read_File(string file_Path);

/**
 * 读取 dexHeader
 * @param dexHeader
 */
void Read_DexHeader(DexHeader* dexHeader);

/**
 * 读取 dexStringId
 * @param dexStringId
 */
void Read_DexStringId(char* fp);

/**
 * 读取 DexTypeId
 * @param fp
 */
void Read_DexTypeId(char* fp);

/**
 * 读取 DexFieldId
 * @param fp
 */
void Read_DexFieldId(char* fp);

/**
 * 读取 DexMethodId
 * @param fp
 */
void Read_DexMethodId(char* fp);

/**
 * 读取 DexProtoId
 * @param fp
 */
void Read_DexProtoId(char* fp);

/**
 * 读取 DexClassDef
 * @param fp
 */
void Read_DexClassDef(char* fp);
/**
 * 取得第一个字节是06，最高位为0，因此没有后继字节，那么取出这个字节里7位有效数据，就是6，也就是说这个字符串是6个字节，但不包括结束字符“\0”。
 * 解码
 * @param pStream
 * @return
 */
DEX_INLINE int readUnsignedLeb128(const u1** pStream);

int main() {

    //得到文件内存地址指针
    char* fp = Read_File("/home/zlq/baidunetdiskdownload/android/classes.dex");
    //char* fp = Read_File("/home/zlq/Desktop/20191013/dy8.3/classes.dex");
    if(!fp)
    {
        printf("空文件!");
        return 0;
    }
    printf("name\t\t\t\t\t\t\t\t\tvalue\t\t\t\t\t\t\t\t\t\t\t\tstart\t\t\tsize\t\tcommet\n");
    //强转为DexFile指针
    DexFile *dex = (DexFile*)&fp;

    //解析struct header_item dex_header
    Read_DexHeader(dex->pHeader);

    // 解析struct string_id_list dex_string_ids
    //Read_DexStringId(fp);

    // 解析struct type_id_list dex_type_ids
    //Read_DexTypeId(fp);

    // 解析struct field_id_list dex_field_ids
    //Read_DexFieldId(fp);

    // 解析struct method_id_list dex_method_ids
    //Read_DexMethodId(fp);

    // 解析struct proto_id_list dex_proto_ids
    //Read_DexProtoId(fp);

    // 解析struct class_def_item_list dex_class_defs
    Read_DexClassDef(fp);
    return 0;
}

void Read_DexClassDef(char* fp)
{
    //u4  classIdx;           /* 描述具体的class类型，值是type_ids的一个index，值必须是一个class类型，不能是数组或者基本类型 */
    //u4  accessFlags;        /* 描述class的访问类型，如public,final,static等 */
    //u4  superclassIdx;      /* 描述父类的类型，值必须是一个class类型，不能是数组或者基本类型 */
    //u4  interfacesOff;      /* 值为偏移地址，被指向的数据结构为type_list，class若没有interfaces，值为0 */
    //u4  sourceFileIdx;      /* 表示源代码文件的信息，值为string_ids的一个index。若此项信息丢失，此项赋值为NO_INDEX=0xFFFFFFFF */
    //u4  annotationsOff;     /* 值为偏移地址，指向的内容是该class的注解，位置在data区，格式为annotations_directory_item，若没有此项，值为0 */
    //u4  classDataOff;       /* 值为偏移地址，指向的内容是该class的使用到的数据，位置在data区，格式为class_data_item。无偶没有此项，则值为0 */
    //u4  staticValuesOff;    /* 值为偏移地址，指向data区里的一个列表，格式为encoded_array_item。若没有此项，值为0. */

    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct class_def_item_list dex_class_defs\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->classDefsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexMethodId));
    printf("\t\t\t数组,存储类的信息");
    //转为DexMethodId结构体
    DexClassDef *dexClassDef = (DexClassDef*)(fp + dex->pHeader->classDefsOff);
    for (int i = 0; i < dex->pHeader->classDefsSize; ++i) {
        printf("\n\tstruct class_def_item class_def[%d]", i);
        int *typeIdsOff = (int*)(fp + dex->pHeader->typeIdsOff);
        int *stringIdsOff = (int*)(fp + dex->pHeader->stringIdsOff);
        const u1* classIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexClassDef->classIdx))];
        //解码
        readUnsignedLeb128(&classIdx_stringdata);
        printf("\n\t\tclassIdx\t\t--> ");
        while(*classIdx_stringdata != '\0')
        {
            printf("%c",(*classIdx_stringdata));
            classIdx_stringdata++;
        }


        printf("\n\t\taccessFlags\t\t-->\t%x",dexClassDef->accessFlags);

        //通过类型去寻找
        const u1* superclassIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexClassDef->superclassIdx))];
        //解码
        readUnsignedLeb128(&superclassIdx_stringdata);
        printf("\n\t\tsuperclassIdx\t--> ");
        while(*superclassIdx_stringdata != '\0')
        {
            printf("%c",(*superclassIdx_stringdata));
            superclassIdx_stringdata++;
        }

        printf("\n\t\tinterfacesOff\t-->\t%d",dexClassDef->interfacesOff);

        if (dexClassDef->sourceFileIdx == -1)
        {
            printf("\n\t\tsourceFileIdx\t-->\tNO_INDEX");
        }
        printf("\n\t\tannotationsOff\t-->\t%d",dexClassDef->annotationsOff);
        printf("\n\t\tclassDataOff\t-->\t%d",dexClassDef->classDataOff);
        printf("\n\t\tstaticValuesOff\t-->\t%d",dexClassDef->staticValuesOff);


        dexClassDef++;
    }
}

void Read_DexProtoId(char* fp)
{
    //u4  shortyIdx;          /* 值为一个string_ids的index号，用来说明该method原型 */
    //u4  returnTypeIdx;      /* 值为一个type_ids的index，表示该method原型的返回值类型 */
    //u4  parametersOff;      /* 指定method原型的参数列表type_list，若method没有参数，则值为0. 参数的格式是type_list */


    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct proto_id_list dex_proto_ids\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->protoIdsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexMethodId));
    printf("\t\t\t数组,函数原型数据索引，记录了方法声明的字符串，返回类型和参数列表");
    //转为DexMethodId结构体
    DexProtoId *dexProtoId = (DexProtoId*)(fp + dex->pHeader->protoIdsOff);
    for (int i = 0; i < dex->pHeader->protoIdsSize; ++i) {
        printf("\n\tstruct proto_id_item proto_id[%d]", i);
        int *typeIdsOff = (int*)(fp + dex->pHeader->typeIdsOff);
        int *stringIdsOff = (int*)(fp + dex->pHeader->stringIdsOff);
        const u1* shortyIdx_stringdata = (u1*)fp+stringIdsOff[dexProtoId->shortyIdx];
        //解码
        readUnsignedLeb128(&shortyIdx_stringdata);
        printf("\n\t\tshortyIdx--> ");
        while(*shortyIdx_stringdata != '\0')
        {
            printf("%c",(*shortyIdx_stringdata));
            shortyIdx_stringdata++;
        }

        const u1* returnTypeIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexProtoId->returnTypeIdx))];
        readUnsignedLeb128(&returnTypeIdx_stringdata);
        printf("\n\t\tprotoIdx--> ");
        while(*returnTypeIdx_stringdata != '\0')
        {
            printf("%c",(*returnTypeIdx_stringdata));
            returnTypeIdx_stringdata++;
        }

        printf("\n\t\tparametersOff--> %d",dexProtoId->parametersOff);

        dexProtoId++;
    }
}


void Read_DexMethodId(char* fp)
{

    //u2  classIdx;           /* method所属的class类型，class_idx的值是type_ids的一个index，必须指向一个class类型 */
    //u2  protoIdx;           /* method的原型，指向proto_ids的一个index */
    //u4  nameIdx;            /* method的名称，值为string_ids的一个index */

    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct method_id_list dex_method_ids\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->methodIdsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexMethodId));
    printf("\t\t\t数组,存储成员函数信息包括函数名 参数和返回值类型");
    //转为DexMethodId结构体
    DexMethodId *dexMethodId = (DexMethodId*)(fp + dex->pHeader->methodIdsOff);
    for (int i = 0; i < dex->pHeader->methodIdsSize; ++i) {
        printf("\n\tstruct method_id_item method_id[%d]", i);
        int *typeIdsOff = (int*)(fp + dex->pHeader->typeIdsOff);
        int *protoIdsOff = (int*)(fp + dex->pHeader->protoIdsOff);
        int *stringIdsOff = (int*)(fp + dex->pHeader->stringIdsOff);
        const u1* classIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexMethodId->classIdx))];
        //解码
        readUnsignedLeb128(&classIdx_stringdata);
        printf("\n\t\tclassIdx--> ");
        while(*classIdx_stringdata != '\0')
        {
            printf("%c",(*classIdx_stringdata));
            classIdx_stringdata++;
        }

        //method的原型，指向proto_ids的一个index
        DexProtoId* dexProtoId = (DexProtoId*) protoIdsOff+dexMethodId->protoIdx;
        const u1* protoIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexProtoId->returnTypeIdx))];
        readUnsignedLeb128(&protoIdx_stringdata);
        printf("\n\t\tprotoIdx--> ");
        while(*protoIdx_stringdata != '\0')
        {
            printf("%c",(*protoIdx_stringdata));
            protoIdx_stringdata++;
        }




        const u1* nameIdx_stringdata = (u1*)fp+stringIdsOff[dexMethodId->nameIdx];
        //解码
        readUnsignedLeb128(&nameIdx_stringdata);
        printf("\n\t\tnameIdx --> ");
        while(*nameIdx_stringdata != '\0')
        {
            printf("%c",(*nameIdx_stringdata));
            nameIdx_stringdata++;
        }


        dexMethodId++;
    }
}

/**
 *
 * @param fp
 */
void Read_DexFieldId(char* fp)
{
    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct field_id_list dex_field_ids\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->fieldIdsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexFieldId));
    printf("\t\t\t数组,存储类的相关的信息");
    //转为DexFieldId结构体
    DexFieldId *dexFieldId = (DexFieldId*)(fp + dex->pHeader->fieldIdsOff);
    for (int i = 0; i < dex->pHeader->fieldIdsSize; ++i) {
        printf("\n\tstruct field_id_item field_id[%d]",i);

        /**
         * 重点:
         *      要想找到field_id的typeIdx对应位置的值 要先找到 typeIdsOff偏移地址+typeIdx等于string_ids的索引,再通过string_ids来寻找值
         *
         *      [类型偏移首地址+typeIdx索引] = 字符串所在的索引位置
         *      字符串偏移首地址[字符串所在的索引位置] =  [类型偏移首地址+typeIdx索引]的具体值
         */
        //拿到类型的偏移首地址
        int *typeIdsOff = (int*)(fp + dex->pHeader->typeIdsOff);
        //拿到字符串的偏移首地址
        int *stringIdsOff = (int*)(fp + dex->pHeader->stringIdsOff);
        //类型偏移地址+索引=字符串索引
        //字符串偏移+字符串索引=typeIdx位置具体的值
        const u1* typeIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexFieldId->typeIdx))];
        //解码
        readUnsignedLeb128(&typeIdx_stringdata);
        printf("\n\t\ttypeIdx --> ");
        while(*typeIdx_stringdata != '\0')
        {
            printf("%c",(*typeIdx_stringdata));
            typeIdx_stringdata++;
        }



        //u2  classIdx;           /* field所属的class类型，class_idx的值时type_ids的一个index，指向所属的类 */
        const u1* classIdx_stringdata = (u1*)fp+stringIdsOff[(*(typeIdsOff+dexFieldId->classIdx))];
        //解码
        readUnsignedLeb128(&classIdx_stringdata);
        printf("\n\t\tclassIdx--> ");
        while(*classIdx_stringdata != '\0')
        {
            printf("%c",(*classIdx_stringdata));
            classIdx_stringdata++;
        }


        //u4  nameIdx;            /* field的名称，它的值是string_ids的一个index */
        const u1* nameIdx_stringdata = (u1*)fp+stringIdsOff[(dexFieldId->nameIdx)];
        //解码
        readUnsignedLeb128(&nameIdx_stringdata);
        printf("\n\t\tnameIdx --> ");
        while(*nameIdx_stringdata != '\0')
        {
            printf("%c",(*nameIdx_stringdata));
            nameIdx_stringdata++;
        }
        dexFieldId++;
    }
}

/**
 *  type_ids 区索引了 .dex 文件里的所有数据类型 ，包括 class 类型 ，数组类型（array types）和基本类型(primitive types) 。 本区域里的元素格式为type_ids_item
 *  type_ids_item 里面 descriptor_idx 的值的意思 ，是 string_ids 里的 index 序号 ，是用来描述此type 的字符串 。
 *  根据 header 里 type_ids_size = 0×07 , type_ids_off = 0xb0 , 找到对应的二进制描述区 。
 *
 *  总结:
 *      descriptor_idx 的值的意思 ，是 string_ids 里的 index 序号 ，是用来描述此type 的字符串 。
 *      字符串指针[descriptor_idx地址内的值] = descriptor_idx值
 *
 * @param fp
 */
void Read_DexTypeId(char* fp)
{
    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct struct type_id_list dex_type_ids\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->typeIdsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexTypeId));
    printf("\t\t\t数组,存储类型相关的信息\n");
    //拿到类型的偏移
    int *typeIdsOff = (int*)(fp + dex->pHeader->typeIdsOff);
    for (int i = 0; i < dex->pHeader->typeIdsSize; ++i) {
        printf("\n\tstruct type_id_list dex_type_ids[%d]:",i);
        //先拿到字符串的偏移
        int *stringIdsOff = (int*)(fp + dex->pHeader->stringIdsOff);
        //再根据类型的偏移去寻找字符串
        const u1* stringdata = (u1*)fp+stringIdsOff[(*typeIdsOff)];
        //解码
        readUnsignedLeb128(&stringdata);
        while(*stringdata != '\0')
        {
            printf("%c",(*stringdata));
            stringdata++;
        }
        //自加 获取下一个指针  int型指针自加 base += 4
        typeIdsOff++;
    }
}

/**
 * 实现方法
 * 总结:
 *      dex->pHeader->stringIdsOff 存储了整个dex的字符串的偏移的首地址 是一个数组类型
 *      dex->pHeader->stringIdsSize 存储字符串的个数
 *      字符串偏移指向的地址的长度通过 readUnsignedLeb128 解码 再判断'\0' 即可
 *      @param dexStringId
 */
void Read_DexStringId(char* fp) {
    DexFile *dex = (DexFile*)&fp;
    printf("\n\nstruct string_id_list dex_string_ids\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x", dex->pHeader->stringIdsSize,(unsigned int) sizeof(DexHeader));
    printf("\t\t\t0x%x", (unsigned int) sizeof(DexStringId));
    printf("\t\t\t数组,元素类型为string_id_item,存储字符串相关的信息");
     //拿到字符串的偏移
    int *p2 = (int*)(fp + dex->pHeader->stringIdsOff);
    ////stringIdsSize -->  DEX中用到的所有字符串内容的大小
    for (int i = 0; i < dex->pHeader->stringIdsSize; ++i) {
        printf("\n\tstruct string_id_item string_id[%d]:",i);
        // 文件首地址+字符串偏移就是字符串存放的位置的第一个数组
        const u1* stringdata = (u1*)fp+*p2;
        //解码
        /*
         * dex文件里采用了变长方式表示字符串长度。一个字符串的长度可能是一个字节（小于256）或者4个字节（1G大小以上）。字符串的长度大多数都是小于 256个字节，因此需要使用一种编码，既可以表示一个字节的长度，也可以表示4个字节的长度，并且1个字节的长度占绝大多数。能满足这种表示的编码方式有很多，但dex文件里采用的是uleb128方式。leb128编码是一种变长编码，每个字节采用７位来表达原来的数据，最高位用来表示是否有后继字节。若第一个 Byte 的最高位为 1 ，则表示还需要下一个 Byte 来描述 ，直至最后一个 Byte 的最高位为 0 。每个 Byte 的其余 Bit 用来表示数据
         */
        readUnsignedLeb128(&stringdata);

        //转码后该字符串为 内存数据为 \0 时 则结束
        while(*stringdata != '\0')
        {
            //换行
            if((*stringdata) != '\a' && (*stringdata) != '\b' &&(*stringdata) != '\t' && (*stringdata) != '\n' && (*stringdata) != '\v' && (*stringdata) != '\r' && (*stringdata) != '\f' )
            {
                printf("%c",(*stringdata));
            }
            //printf("%c",(*stringdata));
            stringdata++;

        }
        p2++;

    }

}

/**
 * 实现方法
 * @param dexHeader
 */
void Read_DexHeader(DexHeader* dexHeader)
{
    //struct header_item dex_header

    printf("struct header_item dex_header\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0x0");
    printf("\t\t\t\t0x%x", (unsigned int)sizeof(DexHeader));
    printf("\t\tDEX 文件头，记录了一些当前文件的信息以及其他数据结构在文件中的偏移量");
    printf("\n\tmagic\t\t\t\t\t\t\t\t");
    char* magic = (char*)dexHeader->magic;
    while(*magic != '\0')
    {
        if(*magic != '\n')
        {
            printf("%c ",(*magic));
        }
        magic++;
    }
    printf("\t\t\t\t\t\t\t\t\t\t0x0\t\t\t\t0x%x\t\t\t取值必须是字符串 \"dex\\n035\\0\" 或者字节byte数组 {0x64 0x65 0x78 0x0a 0x30 0x33 0x35 0x00}",
            (unsigned int)sizeof(dexHeader->magic));
    printf("\n\tchecksum\t\t\t\t\t\t\t0x%X\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t\t0x%x\t\t\t文件内容的校验和,不包括magic和自己,主要用于检查文件是否损坏"
            ,dexHeader->checksum
            ,(unsigned int)sizeof(dexHeader->magic)
            ,(unsigned int)sizeof(dexHeader->checksum));

    printf("\n\tsignature\t\t\t\t\t\t\t");
    for (int i = 0; i < (unsigned int)sizeof(dexHeader->signature); ++i) {
        printf("%02X",dexHeader->signature[i]);
    }

    printf("\t\t\t0x%x\t\t\t\t0x%x\t\t签名信息,不包括 magic,checksum,signature",
            (unsigned int)sizeof(dexHeader->magic)+(unsigned int)sizeof(dexHeader->checksum),
            (unsigned int)sizeof(dexHeader->signature));


    printf("\n\tfileSize\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\t整个文件的长度,单位为字节,包括所有的内容"
            ,dexHeader->fileSize
            ,(unsigned int)(sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->fileSize));


    printf("\n\theaderSize\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\t默认是0x70个字节"
            ,dexHeader->headerSize
            ,(unsigned int)(sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->headerSize));

    printf("\n\tendianTag\t\t\t\t\t\t\t0x%X\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\t大小端标签，标准.dex文件为小端，此项一般固定为0x12345678常量"
            ,dexHeader->endianTag
            ,(unsigned int)(sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->endianTag));

    printf("\n\tlinkSize\t\t\t\t\t\t\t0x%X\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\t链接数据的大小"
            ,dexHeader->linkSize
            ,(unsigned int)(sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->linkSize));

    printf("\n\tlinkOff\t\t\t\t\t\t\t\t0x%X\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\t链接数据的偏移值"
            ,dexHeader->linkOff
            ,(unsigned int)(sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->linkOff));


    printf("\n\tmapOff\t\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tmap item的偏移地址，该item属于data区里的内容，值要大于等于dataOff的大小"
            ,dexHeader->mapOff
            ,(unsigned int)(sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->mapOff));

    printf("\n\tstringIdsSize\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中用到的所有字符串内容的大小"
            ,dexHeader->stringIdsSize
            ,(unsigned int)(sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->stringIdsSize));


    printf("\n\tstringIdsOff\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中用到的所有字符串内容的偏移量"
            ,dexHeader->stringIdsOff
            ,(unsigned int)(sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->stringIdsOff));


    printf("\n\ttypeIdsSize\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中类型数据结构的大小"
            ,dexHeader->typeIdsSize
            ,(unsigned int)(sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->typeIdsSize));


    printf("\n\ttypeIdsOff\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中类型数据结构的偏移值"
            ,dexHeader->typeIdsOff
            ,(unsigned int)(sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->typeIdsOff));

    printf("\n\tprotoIdsSize\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中的元数据信息数据结构的大小"
            ,dexHeader->protoIdsSize
            ,(unsigned int)(sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->protoIdsSize));

    printf("\n\tprotoIdsOff\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中的元数据信息数据结构的偏移值"
            ,dexHeader->protoIdsOff
            ,(unsigned int)(sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->protoIdsOff));

    printf("\n\tfieldIdsSize\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中字段信息数据结构的大小"
            ,dexHeader->fieldIdsSize
            ,(unsigned int)(sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->fieldIdsSize));

    printf("\n\tfieldIdsOff\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中字段信息数据结构的偏移值"
            ,dexHeader->fieldIdsOff
            ,(unsigned int)(sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->fieldIdsOff));

    printf("\n\tmethodIdsSize\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中方法信息数据结构的大小"
            ,dexHeader->methodIdsSize
            ,(unsigned int)(sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->methodIdsSize));

    printf("\n\tmethodIdsOff\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中方法信息数据结构的偏移值"
            ,dexHeader->methodIdsOff
            ,(unsigned int)(sizeof(dexHeader->methodIdsOff)+sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->methodIdsOff));

    printf("\n\tclassDefsSize\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中的类信息数据结构的大小"
            ,dexHeader->classDefsSize
            ,(unsigned int)(sizeof(dexHeader->classDefsSize)+sizeof(dexHeader->methodIdsOff)+sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->classDefsSize));

    printf("\n\tclassDefsOff\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中的类信息数据结构的偏移值"
            ,dexHeader->classDefsOff
            ,(unsigned int)(sizeof(dexHeader->classDefsOff)+sizeof(dexHeader->classDefsSize)+sizeof(dexHeader->methodIdsOff)+sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->classDefsOff));

    printf("\n\tdataSize\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中数据区域的结构信息的大小"
            ,dexHeader->dataSize
            ,(unsigned int)(sizeof(dexHeader->dataSize)+sizeof(dexHeader->classDefsOff)+sizeof(dexHeader->classDefsSize)+sizeof(dexHeader->methodIdsOff)+sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->dataSize));

    printf("\n\tdataOff\t\t\t\t\t\t\t\t%d\t\t\t\t\t\t\t\t\t\t\t\t0x%x\t\t\t0x%x\t\t\tDEX中数据区域的结构信息的偏移值"
            ,dexHeader->dataOff
            ,(unsigned int)(sizeof(dexHeader->dataOff)+sizeof(dexHeader->dataSize)+sizeof(dexHeader->classDefsOff)+sizeof(dexHeader->classDefsSize)+sizeof(dexHeader->methodIdsOff)+sizeof(dexHeader->methodIdsSize)+sizeof(dexHeader->fieldIdsOff)+sizeof(dexHeader->fieldIdsSize)+sizeof(dexHeader->protoIdsOff)+sizeof(dexHeader->protoIdsSize)+sizeof(dexHeader->typeIdsOff)+sizeof(dexHeader->typeIdsSize)+sizeof(dexHeader->stringIdsOff)+sizeof(dexHeader->stringIdsSize)+sizeof(dexHeader->mapOff)+sizeof(dexHeader->linkOff)+sizeof(dexHeader->linkSize)+sizeof(dexHeader->endianTag)+sizeof(dexHeader->headerSize)+sizeof(dexHeader->magic)+sizeof(dexHeader->checksum)+sizeof(dexHeader->signature))
            ,(unsigned int)sizeof(dexHeader->dataOff));
}

/**
 * 实现方法
 * @param file_Path
 * @return
 */
char* Read_File(string file_Path) {
    filebuf *pbuf;
    ifstream filestr;
    long size;
    char *buffer;
    // 要读入整个文件，必须采用二进制打开
    filestr.open(file_Path, ios::binary);
    // 获取filestr对应buffer对象的指针
    pbuf = filestr.rdbuf();

    // 调用buffer对象方法获取文件大小
    size = pbuf->pubseekoff(0, ios::end, ios::in);
    pbuf->pubseekpos(0, ios::in);

    // 分配内存空间
    buffer = new char[size];

    // 获取文件内容
    pbuf->sgetn(buffer, size);

    filestr.close();
    // 输出到标准输出
    //cout.write (buffer,size);

    // delete []buffer;
    return buffer;
}

/**
 * 实现方法
 * @param pStream
 * @return
 */
DEX_INLINE int readUnsignedLeb128(const u1** pStream) {
    const u1* ptr = *pStream;
    int result = *(ptr++);
    if (result > 0x7f) {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) {
                    /*
                     * Note: We don't check to see if cur is out of
                     * range here, meaning we tolerate garbage in the
                     * high four-order bits.
                     */
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }
    *pStream = ptr;
    return result;
}