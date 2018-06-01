#include "ConeTraceShader.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "Library.hpp"

void ConeTraceShader::coneTrace(Volume *volume, float dt) {
    if (sort) {
        volume->sortBoards(Camera::getPosition());
    }

    glDisable(GL_DEPTH_TEST);

    bind();
    bindVolume(volume);

    loadVector(getUniform("lightPos"), Light::spatial.position);

    /* Bind cone tracing params */
    loadInt(getUniform("vctSteps"), vctSteps);
    loadFloat(getUniform("vctConeAngle"), vctConeAngle);
    loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
    loadFloat(getUniform("vctLodOffset"), vctLodOffset);
    loadFloat(getUniform("vctDownScaling"), vctDownScaling);

    /* Bind noise map params */
    loadFloat(getUniform("g_stepSize"), g_stepSize);
    loadFloat(getUniform("g_noiseOpacity"), g_noiseOpacity);
    loadVector(getUniform("g_directional"), g_directional);
    loadInt(getUniform("octaves"), octaves);
    loadFloat(getUniform("frequency"), frequency);
    loadFloat(getUniform("persistence"), persistence);

    		glm::vec4 Octaves[4];
		static float totaltime = 0;
        totaltime += dt*100.f;
		for (int i = 0; i<4; i++)
			{
			Octaves[i].x = -(float)(totaltime*0.001);
			Octaves[i].y = 0;
			Octaves[i].z = 0;
			Octaves[i].w = 0;
			}
	glUniformMatrix4fv(getUniform("g_OctaveOffsets"), 1, GL_FALSE, &Octaves[0].x);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind P V Vi*/
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    for (auto cloudBoard : volume->cloudBoards) {
        /* Cone trace from the camera's perspective */
        loadVector(getUniform("center"), volume->position + cloudBoard.position);
        loadFloat(getUniform("scale"), cloudBoard.scale.x);

        /* Bind M N */
        glm::mat4 M = glm::translate(glm::mat4(1.f), volume->position + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale.x));
        loadMatrix(getUniform("M"), &M);
        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);

        /* Cone trace! */
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
    unbindVolume();
    unbind();

    glEnable(GL_DEPTH_TEST);
}

void ConeTraceShader::bindVolume(Volume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    loadInt(getUniform("volumeTexture"), volume->volId);

    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + TexNoise));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, TexNoise));
    loadInt(getUniform("noisemap"), TexNoise);
    
    loadVector(getUniform("xBounds"), volume->position.x + volume->xBounds);
    loadVector(getUniform("yBounds"), volume->position.y + volume->yBounds);
    loadVector(getUniform("zBounds"), volume->position.z + volume->zBounds);
    loadInt(getUniform("voxelDim"), volume->dimension);
}

void ConeTraceShader::unbindVolume() {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

	struct CHAR4
		{
		char x, y, z, w;
		};
    	float RPercent()
		{
		float ret = (float)((rand() % 20000) - 10000);
		return ret / 10000.0f;
		}
	
		float GetDensity(int x, int y, int z, CHAR4* pTexels, UINT VolumeSize)
		{
		if (x < 0)
			x += VolumeSize;
		if (y < 0)
			y += VolumeSize;
		if (z < 0)
			z += VolumeSize;

		x = x%VolumeSize;
		y = y%VolumeSize;
		z = z%VolumeSize;

		int index = x + y*VolumeSize + z*VolumeSize*VolumeSize;

		return (float)pTexels[index].w / 128.0f;
		}

        	void SetNormal(glm::vec3 Normal, int x, int y, int z, CHAR4* pTexels, UINT VolumeSize)
		{
		if (x < 0)
			x += VolumeSize;
		if (y < 0)
			y += VolumeSize;
		if (z < 0)
			z += VolumeSize;

		x = x%VolumeSize;
		y = y%VolumeSize;
		z = z%VolumeSize;

		int index = x + y*VolumeSize + z*VolumeSize*VolumeSize;

		pTexels[index].x = (char)(Normal.x * 128.0f);
		pTexels[index].y = (char)(Normal.y * 128.0f);
		pTexels[index].z = (char)(Normal.z * 128.0f);
		}
	
	
void ConeTraceShader::genNoise(int VolumeSize=32) {
    		CHAR4* pData = new CHAR4[VolumeSize*VolumeSize*VolumeSize];
		// Generate normals from the density gradient
			
	
		
		for (UINT i = 0; i<VolumeSize*VolumeSize*VolumeSize; i++)
			{
			pData[i].w = (char)(RPercent() * 128.0f);
			}

		// Generate normals from the density gradient
		float heightAdjust = 0.5f;
		glm::vec3 Normal;
		glm::vec3 DensityGradient;
		for (UINT z = 0; z<VolumeSize; z++)
			{
			for (UINT y = 0; y<VolumeSize; y++)
				{
				for (UINT x = 0; x<VolumeSize; x++)
					{
					DensityGradient.x = GetDensity(x + 1, y, z, pData, VolumeSize) - GetDensity(x - 1, y, z, pData, VolumeSize) / heightAdjust;
					DensityGradient.y = GetDensity(x, y + 1, z, pData, VolumeSize) - GetDensity(x, y - 1, z, pData, VolumeSize) / heightAdjust;
					DensityGradient.z = GetDensity(x, y, z + 1, pData, VolumeSize) - GetDensity(x, y, z - 1, pData, VolumeSize) / heightAdjust;


					Normal = normalize(DensityGradient);
					SetNormal(Normal, x, y, z, pData, VolumeSize);
					}
				}
			}
		

		glGenTextures(1, &TexNoise);
		glBindTexture(GL_TEXTURE_3D, TexNoise);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8_SNORM, VolumeSize, VolumeSize, VolumeSize, 0, GL_RGBA, GL_BYTE, pData);
		glGenerateMipmap(GL_TEXTURE_3D);
}