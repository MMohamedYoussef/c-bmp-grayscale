#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t get16(const unsigned char *p) { return (uint16_t)(p[0] | ((uint16_t)p[1] << 8)); }
static uint32_t get32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static int fail(char *error, size_t cap, const char *message) {
    snprintf(error,cap,"%s",message); return 1;
}
static int convert_bmp(FILE *input, FILE *output, char *error, size_t cap) {
    unsigned char *bytes=NULL;
    long length;
    if (fseek(input,0,SEEK_END)!=0 || (length=ftell(input))<0 || fseek(input,0,SEEK_SET)!=0)
        return fail(error,cap,"Cannot seek input BMP");
    if (length<54 || length>64L*1024*1024) return fail(error,cap,"BMP size outside supported range");
    bytes=malloc((size_t)length);
    if (!bytes) return fail(error,cap,"Out of memory");
    if (fread(bytes,1,(size_t)length,input)!=(size_t)length) { free(bytes); return fail(error,cap,"Truncated BMP"); }
    uint32_t dib=get32(bytes+14), offset=get32(bytes+10), width=get32(bytes+18), hraw=get32(bytes+22);
    uint32_t height = hraw & UINT32_C(0x80000000) ? (~hraw)+1 : hraw;
    if (bytes[0]!='B' || bytes[1]!='M' || get32(bytes+2)!=(uint32_t)length ||
        dib<40 || (uint64_t)14+dib>offset || offset>(uint32_t)length ||
        width==0 || width>100000 || height==0 || height>100000 ||
        get16(bytes+26)!=1 || get16(bytes+28)!=24 || get32(bytes+30)!=0) {
        free(bytes); return fail(error,cap,"Unsupported or malformed 24-bit BMP header");
    }
    uint64_t stride=((uint64_t)width*3+3)&~UINT64_C(3);
    if (stride*height>(uint64_t)length-offset) { free(bytes); return fail(error,cap,"Truncated BMP pixel array"); }
    for (uint32_t y=0;y<height;++y) {
        unsigned char *row=bytes+offset+(size_t)(y*stride);
        for (uint32_t x=0;x<width;++x) {
            unsigned char *pixel=row+x*3;
            unsigned gray=(29u*pixel[0]+150u*pixel[1]+77u*pixel[2]+128u)>>8;
            pixel[0]=pixel[1]=pixel[2]=(unsigned char)gray;
        }
    }
    int ok=fwrite(bytes,1,(size_t)length,output)==(size_t)length && fflush(output)==0;
    free(bytes);
    return ok ? 0 : fail(error,cap,"Cannot write output BMP");
}
#ifndef UNIT_TEST
int main(int argc, char **argv) {
    FILE *input,*output; char error[128]; int result;
    if (argc!=3 || strcmp(argv[1],argv[2])==0) {
        fputs("Usage: bmp-grayscale input.bmp output.bmp (different paths)\n",stderr); return 2;
    }
    input=fopen(argv[1],"rb");
    if(!input) { perror(argv[1]); return 2; }
    /* Validate into a temporary stream before creating the destination. */
    FILE *temp=tmpfile();
    if(!temp) { perror("tmpfile"); fclose(input); return 2; }
    result=convert_bmp(input,temp,error,sizeof(error));
    fclose(input);
    if(result) { fprintf(stderr,"BMP error: %s\n",error); fclose(temp); return 1; }
    if(fseek(temp,0,SEEK_SET)!=0) { perror("Cannot rewind temporary output"); fclose(temp); return 1; }
    output=fopen(argv[2],"wbx");
    if(!output) { perror(argv[2]); fclose(temp); return 2; }
    unsigned char buf[8192]; size_t n;
    while((n=fread(buf,1,sizeof(buf),temp))!=0)
        if(fwrite(buf,1,n,output)!=n) { fputs("Output write error\n",stderr); result=1; break; }
    if(ferror(temp)) result=1;
    if(fclose(output)!=0) result=1;
    if(fclose(temp)!=0) result=1;
    if(result && remove(argv[2])!=0) perror("Cannot remove partial output");
    return result;
}
#endif
