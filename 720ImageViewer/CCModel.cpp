#include "CCModel.h"
#include "CCMesh.h"
#include "CCMaterial.h"

CCModel::CCModel()
{
    Reset();
}
CCModel::~CCModel() = default;

void CCModel::UpdateVectors()
{
    // �����µ�ǰ����
    glm::vec3 front;
    front.x = cos(glm::radians(Rotation.y)) * cos(glm::radians(Rotation.x));
    front.y = sin(glm::radians(Rotation.x));
    front.z = sin(glm::radians(Rotation.y)) * cos(glm::radians(Rotation.x));
    Front = glm::normalize(front);
    // �ټ�����������������
    Right = glm::normalize(glm::cross(Front, WorldUp));  // ��׼��
    Up = glm::normalize(glm::cross(Right, Front));
}
glm::mat4 CCModel::GetMatrix() const
{    
    glm::mat4 model(1.0f);
    model = glm::translate(model, Position);
    model = glm::rotate(model, glm::radians(Rotation.x), Right);
    model = glm::rotate(model, glm::radians(Rotation.y), Up);
    model = glm::rotate(model, glm::radians(Rotation.z), Front);
    return model;
}
void CCModel::Reset() {
    Position = glm::vec3(0.0f);
    Rotation = glm::vec3(0.0f);
    UpdateVectors();
}

void CCModel::Render() const
{
    if (!Visible) return;
    if (!Material.IsNullptr()) Material->Use();
    if (!Mesh.IsNullptr()) Mesh->RenderMesh();
}
