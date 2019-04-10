#define PACKAGE_MAX_SIZE        (1024*4)

inline void ASC2HEX_TEST(char * dst, char * src,int len)
{
    int hex;
    int i;
    int di;
    for(i=0;i<len;i++){
        hex=((unsigned char)src[i])>>4;
        di=i*3;
        dst[di]=hex<10?'0'+hex:'A'-10 +hex ;
        hex=((unsigned char)src[i])&0x0F;
        dst[di+1]=hex<10?'0'+hex:'A'-10 +hex ;
        dst[di+2]=' ';
    }
    dst[len*3]=0;
}

