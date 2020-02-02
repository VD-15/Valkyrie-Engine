#pragma once

#include "TransformComponent.h"

namespace vlk
{
	enum class DockType
	{
		TopLeft,
		TopCenter,
		TopRight,
		CenterLeft,
		Center,
		CenterRight,
		BottomLeft,
		BottomCenter,
		BottomRight
	};

	struct UIComponent : public Component<UIComponent>
	{
		UIComponent(IEntity* e, TransformComponent2D* transform);

		TransformComponent2D* transform;

		DockType dock;

		Vector2 offset;
	};
}