#include "stdafx.h"
#include "COpenGLView.h"
#include "CCamera.h"
#include "CCShader.h"

void COpenGLView::CalcNoCameraProjection(CCamera* camera, CCShader* shader) {
	glm::mat4 view(1.0f);
	camera->view = view;
	camera->projection = view;
	glUniformMatrix4fv(shader->viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(shader->projectionLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void COpenGLView::SetCamera(COpenGLView* view, CCamera* camera) {
	if (view->Camera)
		view->Camera->SetView(nullptr);
	view->Camera = camera;
	if (view->Camera)
		view->Camera->SetView(view);
}

void COpenGLView::CalcCameraProjection(CCamera* camera, CCShader* shader) {
	if (camera) {

		//摄像机矩阵变换
		camera->view = camera->GetViewMatrix();
		glUniformMatrix4fv(shader->viewLoc, 1, GL_FALSE, glm::value_ptr(camera->view));
		//摄像机投影
		camera->projection = camera->Projection == CCameraProjection::Perspective ?
			glm::perspective(glm::radians(camera->FiledOfView), (float)camera->GetView()->Width / (float)camera->GetView()->Height,
				camera->ClippingNear,
				camera->ClippingFar) :
			glm::ortho(-camera->OrthographicSize / 2, camera->OrthographicSize / 2,
				-((float)camera->GetView()->Height / (float)camera->GetView()->Width * camera->OrthographicSize / 2),
				((float)camera->GetView()->Height / (float)camera->GetView()->Width * camera->OrthographicSize / 2),
				camera->ClippingNear, camera->ClippingFar);
		glUniformMatrix4fv(shader->projectionLoc, 1, GL_FALSE, glm::value_ptr(camera->projection));
	}
}
