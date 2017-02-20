#include "pixutils.h"

//private methods -> make static
static pixMap* pixMap_init(unsigned char arrayType);
static pixMap* pixMap_copy(pixMap *p);


static pixMap* pixMap_init(unsigned char arrayType){
	//initialize everything to zero except arrayType
	pixMap *p;
	p = malloc(sizeof(pixMap));
	p->image = 0;
	p->imageWidth = 0;
	p->imageHeight = 0;
	
	p->pixArray_arrays = 0;
	p->pixArray_blocks = 0;
	p->pixArray_overlay = 0;

	return p;
}
	
void pixMap_destroy (pixMap **p){
    //free all mallocs and put a zero pointer in *p
    
 	pixMap *this_ptr = *p;
 	
	if(this_ptr->pixArray_arrays) {
		free(this_ptr->pixArray_arrays);
		this_ptr->pixArray_arrays = 0;
	}
	
	if(this_ptr->pixArray_blocks) {
	for(int i = 0; i < this_ptr->imageHeight; i++) {
		if(this_ptr->pixArray_blocks[i]) free(this_ptr->pixArray_blocks[i]);
	}
	if(*p) free(*p);
	this_ptr->pixArray_blocks = 0;
	}
	
	if(this_ptr->pixArray_overlay) {
		//if(this_ptr->pixArray_overlay[0]) free(this_ptr->pixArray_overlay[0]);
	if(*p) free(*p);
	this_ptr->pixArray_overlay = 0;
	}
	
	if(this_ptr->image) {
		free(this_ptr->image);
		this_ptr->image = 0;
	}
	
	//if(new)free(new);
}
	
pixMap *pixMap_read(char *filename,unsigned char arrayType){
 //library call reads in the image into p->image and sets the width and height
	pixMap *p=pixMap_init(arrayType);
 int error;
 if((error=lodepng_decode32_file(&(p->image), &(p->imageWidth), &(p->imageHeight),filename))){
  fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
  return 0;
	}
 //allocate the 2-D rgba arrays
 
	if (arrayType == 0) {
		//can only allocate for the number of rows - each row will be an array of MAXWIDTH
		p->pixArray_arrays = malloc(p->imageHeight * sizeof(rgba[MAXWIDTH]));
		
			//copy each row of the image into each row
			/*
			int n = 0;
			for(int i = 0; i < p->imageHeight; i++) {
				for(int j = 0; j < p->imageWidth; j++) {
					p->pixArray_arrays[i][j] = n++;
				}
			}
			*/ 
	}
		
	else if (arrayType == 1) {
		//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
		p->pixArray_blocks = malloc(p->imageHeight * sizeof(rgba*));
		
			//use a loop allocate a block of memory for each row
			for(int i = 0; i < p->imageHeight; i++) {
				p->pixArray_blocks[i] = malloc(p->imageWidth * sizeof(rgba)); //pixMap
				//memset(p->pixArray_blocks[i], 0, p->imageWidth[i] * sizeof(rbga));
			}
			
			//copy each row of the image into the newly allocated block
			
			/*
			//int n = 0;
			for(int i = 0; i < p->imageHeight; i++) {
				for(int j = 0; j < p->imageWidth; j++) {
					//p->pixArray_blocks[i][j] = n++;
				}
			}
			*/ 
	}
			
	else if (arrayType == 2) {
		//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
		p->pixArray_overlay = malloc(p->imageHeight * sizeof(rgba*));
  
			//set the first pointer to the start of p->image
			p->pixArray_overlay[0] = (rgba*)p->image;
			//each subsequent pointer is the previous pointer + p->imageWidth
			for (int i = 1; i < p->imageHeight; i++) {
				p->pixArray_overlay[i] = p->pixArray_overlay[i - 1] + p->imageWidth;
			}
			
			/*
			//int n = 0;
			for(int i = 0; i < p->imageHeight; i++) {
				for(int j = 0; j < p->imageWidth; j++) {
					//p->pixArray_overlay[i][j] = n++;
				}
			}
			*/ 
			
	}
	else {
		return 0;
	}				
	return p;
}
int pixMap_write(pixMap *p,char *filename){
	int error=0;
	
	//for arrayType 1 and arrayType 2 have to write out a controws  to the image using memcpy
	 if (p->arrayType ==0){
		 /*
		 for (int i=0;i<p->imageHeight;i++){
			for (int j=0;j<p->imageWidth;j++){
		 fprintf(filename,"%c",p->pixArray_arrays[i][j]);
		 */
		 
    //have to copy each row of the array into the corresponding row of the image	
	 }	
		else if (p->arrayType ==1){
   //have to copy each row of the array into the corresponding row of the image		
	 }
	//library call to write the image out 
 if(lodepng_encode32_file(filename, p->image, p->imageWidth, p->imageHeight)){
  fprintf(stderr,"error %u: %s\n", error, lodepng_error_text(error));
  return 1;
	}
	return 0;
}	 

int pixMap_rotate(pixMap *p,float theta){
	pixMap *oldPixMap=pixMap_copy(p);
	if(!oldPixMap)return 1;
	
	const float ox=p->imageWidth/2.0f;
	const float oy=p->imageHeight/2.0f;
	const float s=sin(degreesToRadians(-theta));
	const float c=cos(degreesToRadians(-theta));
	
	for(int y=0;y<p->imageHeight;y++){
		for(int x=0;x<p->imageWidth;x++){ 
			float rotx = c*(x-ox) - s * (oy-y) + ox;
			float roty = -(s*(x-ox) + c * (oy-y) - oy);
			int rotj=rotx+.5;
			int roti=roty+.5; 
		 if(roti >=0 && roti < oldPixMap->imageHeight && rotj >=0 && rotj < oldPixMap->imageWidth){
			 if(p->arrayType==0) memcpy(p->pixArray_arrays[y]+x,oldPixMap->pixArray_arrays[roti]+rotj,sizeof(rgba));
			 else if(p->arrayType==1) memcpy(p->pixArray_blocks[y]+x,oldPixMap->pixArray_blocks[roti]+rotj,sizeof(rgba));
			 else if(p->arrayType==2) memcpy(p->pixArray_overlay[y]+x,oldPixMap->pixArray_overlay[roti]+rotj,sizeof(rgba));			 
			}
			else{
				if(p->arrayType==0) memset(p->pixArray_arrays[y]+x,0,sizeof(rgba));
				else if(p->arrayType==1) memset(p->pixArray_blocks[y]+x,0,sizeof(rgba));
				else if(p->arrayType==2) memset(p->pixArray_overlay[y]+x,0,sizeof(rgba));		
			}		
		}	
	}
 pixMap_destroy(&oldPixMap);
 return 0;
}

pixMap *pixMap_copy(pixMap *p){
	pixMap *new=pixMap_init(p->arrayType);
	//allocate memory for new image of the same size a p->image and copy the image
	new->image = malloc(p->imageHeight * p->imageWidth * sizeof(rgba));
	
	//allocate memory and copy the arrays. 
	if (new->arrayType ==0){
		//insert code
		new->pixArray_arrays = malloc(p->imageHeight * sizeof(rgba[MAXWIDTH]));
		memcpy(new->pixArray_arrays, p->pixArray_arrays, p->imageHeight * sizeof(rgba[MAXWIDTH]));
		//for(int i = 0; i < p->imageHeight; i++)
		//	memcpy(new->pixArray_arrays[i], p->pixArray_arrays[i], p->imageWidth * sizeof(pixMap));
	}	
 else if (new->arrayType ==1){
		//insert code
		new->pixArray_blocks = malloc(p->imageHeight * sizeof(rgba*));
		for(int i = 0; i < p->imageHeight; i++)
			new->pixArray_blocks[i] = malloc(p->imageWidth * sizeof(rgba));
		for(int i = 0; i < p->imageHeight; i++){
			for(int j = 0; j < p->imageWidth; j++){
				new->pixArray_blocks[i][j] = p->pixArray_blocks[i][j];
			 }
		 }
		 //for(int i = 0;i < new->imageHeight; i++)
		//	memcpy(new->pixArray_blocks[i], p->pixArray_blocksa[i], p->imageWidth * sizeof(rgba));
	}
	else if (new->arrayType ==2){
		//insert code
		new->pixArray_overlay = malloc(p->imageHeight * sizeof(rgba*));
				
		new->pixArray_overlay[0] = (rgba*)new->image;
		//new->pixArray_overlay[0] = malloc(p->imageHeight * p->imageWidth * sizeof(rgba));
			
		for (int i = 1; i < p->imageHeight; i++) 
			new->pixArray_overlay[i] = new->pixArray_overlay[i - 1] + p->imageWidth;
		/*	
		for(int i=0; i < p->imageHeight; i++){
			for(int j=0; j < p->imageWidth; j++){
				new->pixArray_overlay[i][j] = p->pixArray_overlay[i][j];
			}
		}
		*/ 
		//memcpy(new->pixArray_overlay, p->pixArray_overlay, p->imageHeight * p->imageWidth * sizeof(rgba));
		memcpy(new->image, p->image, p->imageHeight * p->imageWidth * sizeof(rgba));
	}
	return new;
}	
