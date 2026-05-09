#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 WorldPos;

uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;
uniform sampler2D texture1;

uniform bool uTerrain = false;
uniform bool uUnlit   = false;
uniform bool uViewmodel = false;

// Lighting
uniform vec3 uSunDir       = vec3(-0.3, 0.9, -0.2);
uniform vec3 uSunColor     = vec3(0.85, 0.85, 0.92);
uniform vec3 uAmbientSky   = vec3(0.55, 0.60, 0.70);
uniform vec3 uAmbientGround= vec3(0.20, 0.20, 0.22);

// Fog (now exponential — much more visible/atmospheric)
uniform vec3  uFogColor    = vec3(0.62, 0.65, 0.70);
uniform float uFogDensity  = 0.012;       // högre = mer "dense" fog
uniform float uFogHeightFalloff = 0.04;   // fog thins with altitude

uniform vec3 uCameraPos = vec3(0.0);

float band(float v, float lo, float hi) { return smoothstep(lo, hi, v); }

vec3 desaturate(vec3 c, float amount) {
    float gray = dot(c, vec3(0.299, 0.587, 0.114));
    return mix(c, vec3(gray), amount);
}

void main() {
    if (uUnlit) {
        FragColor = texture(texture1, TexCoord);
        return;
    }

    vec3 N = normalize(Normal);
    vec3 L = normalize(uSunDir);
    vec3 albedo;

    if (uTerrain) {
        vec3 grass = texture(grassTex, TexCoord).rgb;
        vec3 rock  = texture(rockTex,  TexCoord).rgb;
        vec3 snow  = texture(snowTex,  TexCoord).rgb;

        float h = WorldPos.y;
        float grassW = 1.0 - band(h, 2.0, 8.0);
        float snowW  = band(h, 16.0, 22.0);
        float rockW  = max(1.0 - grassW - snowW, 0.0);

        float steep = 1.0 - smoothstep(0.55, 0.80, N.y);
        rockW  = max(rockW, steep);
        grassW *= (1.0 - steep);
        snowW  *= (1.0 - steep);

        float total = grassW + rockW + snowW + 1e-5;
        grassW /= total; rockW /= total; snowW /= total;

        albedo = grass * grassW + rock * rockW + snow * snowW;
    } else {
        albedo = texture(texture1, TexCoord).rgb;
    }

    // Hemisphere ambient
    float upness  = N.y * 0.5 + 0.5;
    vec3  ambient = mix(uAmbientGround, uAmbientSky, upness);

    // Wrapped Lambert (softer overcast terminator)
    float NdotL = dot(N, L);
    float wrap  = max((NdotL + 0.3) / 1.3, 0.0);
    vec3 diffuse = uSunColor * wrap;

    // Rim light
    vec3 V = normalize(uCameraPos - WorldPos);
    float rim = 1.0 - max(dot(N, V), 0.0);
    rim = pow(rim, 2.5);
    vec3 rimLight = uAmbientSky * rim * 0.35;

    vec3 color = albedo * (ambient + diffuse) + rimLight;

    // ---- Exponential height-based fog ----
    // Thicker near ground, thins out at altitude — proper Souls atmosphere.
    float dist = length(WorldPos - uCameraPos);
    float heightFactor = exp(-uFogHeightFalloff * max(WorldPos.y, 0.0));
    if (!uViewmodel) {
      float fogAmount = 1.0 - exp(-dist * uFogDensity * heightFactor);
      fogAmount = clamp(fogAmount, 0.0, 1.0);
      color = mix(color, uFogColor, fogAmount);
    }
    // Souls flavor
    color = desaturate(color, 0.15);
    color *= vec3(0.96, 0.98, 1.02);

    // Tone-map + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
