#include "UIMapEditorHelpWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeBackButton.h"

UIMapEditorHelpWindow::UIMapEditorHelpWindow (IFrontend *frontend) :
		UIHelpWindow(UI_WINDOW_MAPEDITOR_HELP, frontend, WINDOW_FLAG_MODAL)
{
	_iconSize = 0.06f;
	UINodeBackground *background = new UINodeBackground(frontend, tr("Help"), false);
	background->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	add(background);

	const float padding = std::max(_currentX, _currentY);
	const float yGap = 0.08f;
	const float top = background->getY() / static_cast<float>(_frontend->getHeight()) + padding;
	const float middle = background->getMiddle() + padding;
	const float left = background->getLeft() + padding;
	_currentX = middle;
	_currentY = top;

	{
		const AutoBorder b(this, _currentY);
		addKey("+");
		addTexture("icon-result");
		addString(tr("Increase map size"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey("-");
		addTexture("icon-result");
		addString(tr("Decrease map size"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey("SHIFT");
		addTexture("icon-plus");
		addKey("+");
		addTexture("icon-result");
		addString(tr("Increase map size"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey("-");
		addTexture("icon-result");
		addString(tr("Decrease map size"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CURSOR"));
		addTexture("icon-result");
		addString(tr("Scroll map"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey("s");
		addTexture("icon-result");
		addString(tr("Save map"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SPACE"));
		addTexture("icon-result");
		addString(tr("Rotate entity"));
		_currentY += yGap;
	}

	_currentX = left;
	_currentY = top;
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("MOUSE WHEEL"));
		addTexture("icon-result");
		addString(tr("Zoom map"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("LMB"));
		addTexture("icon-result");
		addString(tr("Place tile"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("MMB"));
		addTexture("icon-result");
		addString(tr("Select tile"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("RMB"));
		addTexture("icon-result");
		addString(tr("Remove tile"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("z");
		addTexture("icon-result");
		addString(tr("Undo"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("y");
		addTexture("icon-result");
		addString(tr("Redo"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("ALT"));
		addTexture("icon-plus");
		addKey(tr("Mouse movement"));
		addTexture("icon-result");
		addString(tr("Shift tile"));
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey(tr("CURSOR"));
		addTexture("icon-result");
		addString(tr("Shift map (cursor direction)"));
		_currentY += yGap;
	}

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIMapEditorHelpWindow::~UIMapEditorHelpWindow ()
{
}
