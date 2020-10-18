#include "CCMesh.h"
#include "CCRenderGlobal.h"
#include "CCMaterial.h"
#include "CCMeshLoader.h"

CCMesh::CCMesh()
{

}
CCMesh::~CCMesh()
{
	ReleaseBuffer();
}

void CCMesh::GenerateBuffer()
{
	CCRenderGlobal* info = CCRenderGlobal::GetInstance();

	size_t positions_size = position.size();
	size_t uv_size = texCoords.size();
	size_t normals_size = normals.size();
	size_t vertices_temp_size = (2 * positions_size) * 3 + positions_size * 2;
	GLfloat* vertices_temp = new GLfloat[vertices_temp_size];
	for (size_t i = 0, ui = 0; i < positions_size; i++) {
		glm::vec3 vertex = position[i];
		glm::vec3 normal = i < normals_size ? normals[i] : glm::vec3(0.0f);
		glm::vec2 uvi = i < uv_size ? texCoords[i] : glm::vec2(0.0f);
		vertices_temp[ui++] = vertex.x;
		vertices_temp[ui++] = vertex.y;
		vertices_temp[ui++] = vertex.z;
		vertices_temp[ui++] = normal.x;
		vertices_temp[ui++] = normal.y;
		vertices_temp[ui++] = normal.z;
		vertices_temp[ui++] = uvi.x;
		vertices_temp[ui++] = uvi.y;
	}

	//VBO
	glGenBuffers(1, &MeshVBO);
	glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_temp_size * sizeof(GLfloat), vertices_temp, GL_STATIC_DRAW);
	glBindVertexArray(MeshVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete []vertices_temp;

	//EBO
	glGenBuffers(1, &MeshEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLint), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void CCMesh::ReleaseBuffer()
{
	if (MeshEBO > 0) glDeleteBuffers(1, &MeshEBO);
	if (MeshVBO > 0) glDeleteBuffers(1, &MeshVBO);
}

void CCMesh::RenderMesh()
{
	if (MeshEBO > 0 && MeshVBO > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshEBO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
}

void CCMesh::LoadFromObj(const wchar_t* path)
{
	CCMeshLoader::GetMeshLoaderByType(MeshTypeObj)->Load(path, this);
}
void CCMesh::UnLoad()
{
	position.clear();
	normals.clear();
	indices.clear();
	texCoords.clear();

	ReleaseBuffer();
}

