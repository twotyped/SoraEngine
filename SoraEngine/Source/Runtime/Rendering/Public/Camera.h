#pragma once

#include "Runtime/Rendering/RenderingTypes.h"
#include "Runtime/RHI/Public/RHIDevice.h"
#include <memory>

namespace Sora::Rendering
{

	struct CameraDesc {
		float FOV = 60.0f;
		float AspectRatio = 16.0f / 9.0f;
		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;
	};

	class Camera {
	public:
		explicit Camera(CameraDesc InCameraDesc) : Desc(InCameraDesc) {}
		~Camera() = default;

		void SetPosition(const Vec3& InPosition) {
			Position = InPosition;
			bViewDirty = true;
		}

		void Move(const Vec3& InTranslation) {
			Position += InTranslation;
			bViewDirty = true;
		}

		void SetOrientation(float InPitchDegrees, float InYawDegrees, float InRollDegrees = 0.0f) {
			Quaternion qPitch = glm::angleAxis(glm::radians(InPitchDegrees), Vec3(1.0f, 0.0f, 0.0f));
			Quaternion qYaw   = glm::angleAxis(glm::radians(InYawDegrees),   Vec3(0.0f, 1.0f, 0.0f));
			Quaternion qRoll  = glm::angleAxis(glm::radians(InRollDegrees),  Vec3(0.0f, 0.0f, 1.0f));
			
			Orientation = glm::normalize(qYaw * qPitch * qRoll);
			bViewDirty = true;
		}

		void Rotate(float InPitchDegrees, float InYawDegrees, float InRollDegrees = 0.0f) {
			Quaternion qPitch = glm::angleAxis(glm::radians(InPitchDegrees), GetRightVector());
			Quaternion qYaw   = glm::angleAxis(glm::radians(InYawDegrees),   Vec3(0.0f, 1.0f, 0.0f));
			
			Orientation = glm::normalize(qYaw * qPitch * Orientation);
			bViewDirty = true;
		}

		void SetPerspective(float InFovDegrees, float InAspectRatio, float InNearPlane, float InFarPlane) {
			Desc.FOV = InFovDegrees;
			Desc.AspectRatio = InAspectRatio;
			Desc.NearPlane = InNearPlane;
			Desc.FarPlane = InFarPlane;
			bProjectionDirty = true;
		}

		void SetAspectRatio(float InAspectRatio) {
			Desc.AspectRatio = InAspectRatio;
			bProjectionDirty = true;
		}

		const Mat4& GetViewMatrix() const {
			UpdateUBO();
			return UBO.ViewMatrix;
		}

		const Mat4& GetProjectionMatrix() const {
			UpdateUBO();
			return UBO.ProjectionMatrix;
		}

		const Mat4& GetViewProjectionMatrix() const {
			UpdateUBO();
			return UBO.ViewProjectionMatrix;
		}

		const UniformBufferObject& GetUBO() const {
			UpdateUBO();
			return UBO;
		}

		Vec3 GetForwardVector() const { return glm::rotate(Orientation, Vec3(0.0f, 0.0f, -1.0f)); }
		Vec3 GetUpVector() const      { return glm::rotate(Orientation, Vec3(0.0f, 1.0f, 0.0f)); }
		Vec3 GetRightVector() const   { return glm::rotate(Orientation, Vec3(1.0f, 0.0f, 0.0f)); }
		Vec3 GetPosition() const      { return Position; }
		const CameraDesc& GetDesc() const	 { return Desc; }

		void UpdateUBO() const {
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
				UBO.ProjectionMatrix[1][1] *= -1.0f; // TODO: Make this platform-agnostic for Vulkan, DirectX, and Metal later.
				bProjectionDirty = false;
			}

			UBO.ViewProjectionMatrix = UBO.ProjectionMatrix * UBO.ViewMatrix;
		}
		
	private:
		Vec3 Position{ 0.0f, 0.0f, 0.0f };
		Quaternion Orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
		CameraDesc Desc{};

		mutable UniformBufferObject UBO{};
		mutable bool bViewDirty = true;
		mutable bool bProjectionDirty = true;
	};

} // namespace Sora
