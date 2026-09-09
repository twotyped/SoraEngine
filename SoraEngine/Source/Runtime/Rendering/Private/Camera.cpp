#include "Runtime/Rendering/Public/Camera.h"

namespace Sora::Rendering
{

	void Camera::SetPosition(const Vec3& InPosition) {
		Position = InPosition;
		bViewDirty = true;
	}

	void Camera::Move(const Vec3& InTranslation) {
		Position += InTranslation;
		bViewDirty = true;
	}

	void Camera::SetOrientation(const Quaternion& InOrientation) {
		Orientation = glm::normalize(InOrientation);
		bViewDirty = true;
	}

	void Camera::Rotate(const Quaternion& InDeltaRotation) {
		SetOrientation(InDeltaRotation * Orientation);
	}

	void Camera::SetOrientation(float InPitchDegrees, float InYawDegrees, float InRollDegrees) {
		Quaternion qPitch = glm::angleAxis(glm::radians(InPitchDegrees), Vec3(1.0f, 0.0f, 0.0f));
		Quaternion qYaw   = glm::angleAxis(glm::radians(InYawDegrees),   Vec3(0.0f, 1.0f, 0.0f));
		Quaternion qRoll  = glm::angleAxis(glm::radians(InRollDegrees),  Vec3(0.0f, 0.0f, 1.0f));
		
		SetOrientation(qYaw * qPitch * qRoll);
	}

	void Camera::Rotate(float InPitchDegrees, float InYawDegrees, float InRollDegrees) {
		Quaternion qPitch = glm::angleAxis(glm::radians(InPitchDegrees), GetRightVector());
		Quaternion qYaw   = glm::angleAxis(glm::radians(InYawDegrees),   Vec3(0.0f, 1.0f, 0.0f));
		Quaternion qRoll  = glm::angleAxis(glm::radians(InRollDegrees),  GetForwardVector());
		
		Rotate(qYaw * qPitch * qRoll);
	}

	void Camera::SetPerspective(float InFovDegrees, float InAspectRatio, float InNearPlane, float InFarPlane) {
		Desc.FOV = InFovDegrees;
		Desc.AspectRatio = InAspectRatio;
		Desc.NearPlane = InNearPlane;
		Desc.FarPlane = InFarPlane;
		bProjectionDirty = true;
	}

	void Camera::SetAspectRatio(float InAspectRatio) {
		Desc.AspectRatio = InAspectRatio;
		bProjectionDirty = true;
	}

	const Mat4& Camera::GetViewMatrix() const {
		UpdateUBO();
		return UBO.ViewMatrix;
	}

	const Mat4& Camera::GetProjectionMatrix() const {
		UpdateUBO();
		return UBO.ProjectionMatrix;
	}

	const Mat4& Camera::GetViewProjectionMatrix() const {
		UpdateUBO();
		return UBO.ViewProjectionMatrix;
	}

	const UniformBufferObject& Camera::GetUBO() const {
		UpdateUBO();
		return UBO;
	}

	void Camera::UpdateUBO() const {
		if (!bViewDirty && !bProjectionDirty) {
			return;
		}

		if (bViewDirty) {
			Mat4 Translation = glm::translate(Mat4(1.0f), -Position);
			Mat4 Rotation = glm::toMat4(glm::conjugate(Orientation));

			UBO.ViewMatrix = Rotation * Translation;
			bViewDirty = false;
		}

		if (bProjectionDirty) {
			UBO.ProjectionMatrix = Perspective(glm::radians(Desc.FOV), Desc.AspectRatio, Desc.NearPlane, Desc.FarPlane);
			bProjectionDirty = false;
		}

		UBO.ViewProjectionMatrix = UBO.ProjectionMatrix * UBO.ViewMatrix;
	}

	void Camera::LookAt(const Vec3& InTarget, const Vec3& InUp) {
		Vec3 Forward = glm::normalize(InTarget - Position);
		
		if (glm::length(Forward) < 0.0001f) {
			return;
		}

		Orientation = glm::quatLookAt(Forward, InUp);
		bViewDirty = true;
	}

} // namespace Sora::Rendering