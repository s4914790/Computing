#version 420 core
// Base code taken from environment, objviewer, shadowmap examples from John Macey, mostly
// Vertex Shader Attributes



// Attributes passed on from the vertex shader
smooth in vec3 FragmentPosition;
smooth in vec3 FragmentNormal;
smooth in vec2 FragmentTexCoord;


/// @brief our output fragment colour
out vec4 FragColour;

// Level of Detail
uniform int envMaxLOD = 8;
// Environment Map
uniform samplerCube envMap;
// Metal base color texture
uniform sampler2D albedoMap;
// Dirt
uniform sampler2D dirtMap;
// Gloss or roughness texture
uniform sampler2D glossMap;
// Specular Highlight Texture
uniform sampler2D specMap;
// Normal Map Texture
uniform sampler2D normalMap;
// Base material
uniform mat4 invV;
// Uniform reflection factor
const float reflect_factor = 0.5;
// true = reflect, false = refract
uniform bool isReflect = true;
// refractive index
uniform float refractiveIndex = 1.0;


mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to normalboy.
  */
vec3 rotateVector(vec3 src, vec3 normalboy, vec3 vec) {
    float angle = acos(dot(src,normalboy));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,normalboy));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}

// set important material values
uniform float F0 = 0.5; // fresnel reflectance at normal incidence

void main (void)
{
    // Calculate the normal (this is the expensive bit in Phong)
    vec3 nt = normalize( FragmentNormal );

    // Calculate the eye vector
    vec3 vt = normalize(vec3(-FragmentPosition));

    vec3 lookup;

    if (isReflect) {
        lookup = reflect(vt,nt);
    } else {
        lookup = refract(vt,nt,refractiveIndex);
    }

    vec4 test = texture(albedoMap, FragmentTexCoord);
    vec4 colour = texture(envMap, FragmentPosition).rgba;

    // environment map reflections
    //vec4 colour = textureLod(envMap, lookup, gloss*2);

    FragColour = test;
}
