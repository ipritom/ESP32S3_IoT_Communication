#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
This transform  hex string frame like "5A 1B" to uint8_t array as hex value
It puts the data to the given array destination frame

buffer : source array 
frame: destination array

Returns:
    int length : length of the frame
*/
int frame_str2hex(char buffer[], uint8_t *frame){
    
    char temp[5]; // temp array for hex segment
    uint8_t temp_pos=0; // temp array index
    uint8_t frame_pos=0; // frame array index (frame_pos +1 is the legth of the frame)
    char *ptr = buffer; // buffer array pointer
    while (*ptr!='\0')
    {   
        if (*ptr==32){
            temp[temp_pos] = '\0';
            // printf("cmd : %s ", temp);
            frame[frame_pos] = strtol(temp, NULL, 16);
            // printf("\nframe[%d] = %d\n", frame_pos, frame[frame_pos]);
            temp_pos = 0;
            frame_pos++;
            ptr++;
            continue;
        }
        temp[temp_pos] = *ptr;
        temp_pos++;
        ptr++;

    }

    if(temp_pos!=0){
        temp[temp_pos] = '\0';
        frame[frame_pos] = strtol(temp, NULL, 16);
    }
    

    return frame_pos+1; // return the length of the frame
}