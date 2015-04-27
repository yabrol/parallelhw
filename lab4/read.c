#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifndef ALLOC
#define ALLOC(ptr, type, n) { \
    ptr = (type *)malloc((n)*sizeof(type)); \
    if (!ptr) { \
	fprintf(stderr, "pic_alloc: Can't allocate %d bytes of memory, aborting\n", n); \
	exit(1); \
    } \
}
#endif

typedef unsigned char Pixel1;

typedef struct {		/* PICTURE */
    int nx, ny;			/* width & height, in pixels */
    int bpp;			/* bytes per pixel = 1, 3, or 4 */
    //Pixel1 *pix;		/* array of pixels */
    Pixel1 **pixels;
} Pic;

Pic *pic_alloc(int nx, int ny, int bytes_per_pixel) {
    Pic *p;
    int size = ny*nx*bytes_per_pixel;
    int i;
    ALLOC(p, Pic, 1);
    p->nx = nx;
    p->ny = ny;
    p->bpp = bytes_per_pixel;
	//ALLOC(p->pix, Pixel1, size);
    p->pixels = (Pixel1 **)malloc(p->ny * sizeof(Pixel1 *));
    for (i=0; i<p->ny; i++)
         p->pixels[i] = (Pixel1 *)malloc(p->nx * p->bpp * sizeof(Pixel1));
    return p;
}

void pic_free(Pic *p) {
    //free(p->pix);
    free(p->pixels);
    free(p);
}


char *ppm_get_token(FILE *fp, char *tok, int len) {
    char *t;
    int c;

    for (;;) {			/* skip over whitespace and comments */
	while (isspace(c = getc(fp)));
	if (c!='#') break;
	do c = getc(fp); while (c!='\n' && c!=EOF);	/* gobble comment */
	if (c==EOF) break;
    }

    t = tok;
    if (c!=EOF) {
	do {
	    *t++ = c;
	    c = getc(fp);
	} while (!isspace(c) && c!='#' && c!=EOF && t-tok<len-1);
	if (c=='#') ungetc(c, fp);	/* put '#' back for next time */
    }
    *t = 0;
    return tok;
}

Pic *ppm_read(char *file) {
    FILE *fp;
    char tok[20];
    int nx, ny, pvmax;
    int i;
    Pic *p;

    /* open PPM file */
    if ((fp = fopen(file, "r")) == NULL) {
	fprintf(stderr, "can't read PPM file %s\n", file);
	return 0;
    }

    /* read PPM header */
    if (strcmp(ppm_get_token(fp, tok, sizeof tok), "P6")) {
	fprintf(stderr, "%s not a valid binary PPM file, bad magic#\n", file);
	fclose(fp);
	return 0;
    }
    if (sscanf(ppm_get_token(fp, tok, sizeof tok), "%d", &nx) != 1 ||
	sscanf(ppm_get_token(fp, tok, sizeof tok), "%d", &ny) != 1 ||
	sscanf(ppm_get_token(fp, tok, sizeof tok), "%d", &pvmax) != 1) {
	    fprintf(stderr, "%s is not a valid PPM file: bad size\n", file);
	    fclose(fp);
	    return 0;
    }

    if (pvmax!=255) {
	fprintf(stderr, "%s does not have 8-bit components: pvmax=%d\n",
	    file, pvmax);
	fclose(fp);
	return 0;
    }

    p = pic_alloc(nx, ny, 3);
    printf("reading PPM file %s: %dx%d pixels\n", file, p->nx, p->ny);

    /*
    if (fread(p->pix, p->nx*3, p->ny, fp) != p->ny) {	
	   fprintf(stderr, "premature EOF on file %s\n", file);
	   free(p);
	   fclose(fp);
	   return 0;
    }
    */
    /* read pixels */
    for(i=0;i< p->ny; i++){
        fread(p->pixels[i], p->nx*p->bpp, 1, fp);
    }

    fclose(fp);
    return p;
}

void write_ppm

int main(){
 
 Pic *p = ppm_read("lenn.ppm");
 int i,j;
 printf("Image\n");
 for(i=0;i< p->ny;i++){
    for(j=0;j< p->nx * p->bpp; j++)
        printf("%c",p->pixels[i][j] );
    printf("\n");
 }
 
}