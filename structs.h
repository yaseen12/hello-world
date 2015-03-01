#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct
{
	int x, y, width, height;
} BoxCoordinates;

typedef struct
{
	float fps;
	int size[4]; // rows, cols, channels, #frames
} VideoProperties;

#endif
