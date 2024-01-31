#version 330 core

in vec4 fragPos;
in vec3 N;

out vec4 fragColor;

void main(void)
{
    int x = int((fragPos.x + -3.15) * 0.6) % 2;
    int y = int((fragPos.y + -3.15) * 0.6) % 2;
    int z = int((fragPos.z + -3.15) * 0.6) % 2;

    //int x = int(fragPos.x + 7) % 2;
    //int y = int(fragPos.y + 7) % 2;
    //int z = int(fragPos.z + 7) % 2;

    int xorXY;
    if(x != y) xorXY = 1;
    else xorXY = 0;
    if(xorXY != z) fragColor = vec4(0,0,0,1);
    else fragColor = vec4(1,1,1,1);

}
