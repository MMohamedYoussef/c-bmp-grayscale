#define UNIT_TEST
#include "main.c"
#include <stdlib.h>
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "CHECK failed at line %d: %s\n", __LINE__, #condition); exit(1); } } while (0)

static void put16(unsigned char *p, unsigned value) { p[0]=(unsigned char)value; p[1]=(unsigned char)(value>>8); }
static void put32(unsigned char *p, unsigned value) { for(int i=0;i<4;i++) p[i]=(unsigned char)(value>>(8*i)); }
int main(void) {
    unsigned char bmp[58] = {0}, result[58];
    char error[128];
    FILE *in = tmpfile(), *out = tmpfile(), *bad = tmpfile();
    CHECK(in && out && bad);
    bmp[0]='B'; bmp[1]='M'; put32(bmp+2,58); put32(bmp+10,54);
    put32(bmp+14,40); put32(bmp+18,1); put32(bmp+22,1);
    put16(bmp+26,1); put16(bmp+28,24); put32(bmp+34,4);
    bmp[54]=0; bmp[55]=0; bmp[56]=255;
    CHECK(fwrite(bmp,1,58,in)==58); rewind(in);
    CHECK(convert_bmp(in,out,error,sizeof(error))==0);
    rewind(out); CHECK(fread(result,1,58,out)==58);
    CHECK(result[54]==77 && result[55]==77 && result[56]==77 && result[57]==0);
    CHECK(fwrite(bmp,1,40,bad)==40); rewind(bad);
    CHECK(convert_bmp(bad,out,error,sizeof(error))!=0);
    FILE *unsupported=tmpfile();
    CHECK(unsupported);
    put16(bmp+28,32);
    CHECK(fwrite(bmp,1,58,unsupported)==58); rewind(unsupported);
    CHECK(convert_bmp(unsupported,out,error,sizeof(error))!=0);
    fclose(in); fclose(out); fclose(bad); fclose(unsupported);
    unsigned char top[70]={0}, converted[70];
    FILE *top_input=tmpfile(), *top_output=tmpfile();
    CHECK(top_input && top_output);
    top[0]='B'; top[1]='M'; put32(top+2,70); put32(top+10,54);
    put32(top+14,40); put32(top+18,2); put32(top+22,UINT32_C(0xfffffffe));
    put16(top+26,1); put16(top+28,24); put32(top+34,16);
    top[54]=255; top[57]=255; top[58]=255; top[59]=255;
    top[60]=0xA5; top[61]=0x5A;
    top[64]=255; top[66]=255;
    CHECK(fwrite(top,1,70,top_input)==70); rewind(top_input);
    CHECK(convert_bmp(top_input,top_output,error,sizeof(error))==0);
    rewind(top_output); CHECK(fread(converted,1,70,top_output)==70);
    CHECK(converted[54]==29 && converted[57]==255);
    CHECK(converted[60]==0xA5 && converted[61]==0x5A);
    CHECK(converted[62]==77 && converted[65]==149);
    fclose(top_input); fclose(top_output);
    puts("bmp tests passed");
    return 0;
}
