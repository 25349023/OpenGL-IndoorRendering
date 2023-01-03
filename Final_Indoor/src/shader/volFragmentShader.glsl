#version 430 core

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D scene;
uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform float sampleWeight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D scatterMap;
uniform bool enable;

const int NUM_SAMPLES = 100;

void main()
{
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    if (enable) {
        vec2 deltaTextCoord = vec2(texCoords - lightPositionOnScreen);
        vec2 textCoo = texCoords;
        deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
        float illuminationDecay = 1.0;

        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            textCoo -= deltaTextCoord;
            vec4 samplePoint = texture(scatterMap, textCoo) * sampleWeight;

            samplePoint *= illuminationDecay * weight;

            fragColor += samplePoint;

            illuminationDecay *= decay;
        }
        fragColor *= exposure;
    }
    fragColor += vec4(texture(scene, texCoords).rgb, 1.0);
}
