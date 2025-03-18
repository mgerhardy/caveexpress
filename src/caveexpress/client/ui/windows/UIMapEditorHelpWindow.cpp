#include "UIMapEditorHelpWindow.h"
#include "common/IFrontend.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeBackButton.h"

namespace caveexpress {

UIMapEditorHelpWindow::UIMapEditorHelpWindow (IFrontend *frontend) :
		UIHelpWindow(UI_WINDOW_MAPEDITOR_HELP, frontend, WINDOW_FLAG_MODAL)
{
	UINodeBackground *background = new UINodeBackground(frontend, tr("Help"), false);
	background->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	add(background);

	const float padding = std::max(_currentX, _currentY);
	const float top = background->getY() / static_cast<float>(_frontend->getHeight()) + padding;
	const float middle = background->getMiddle() + padding;
	const float left = background->getLeft() + padding;

	_iconSize = 0.03f;
	const float yGap = 0.05f, yGap2 = yGap * 2.f;
	const std::string& font = HUGE_FONT;

	_currentX = left;
	_currentY = top;
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("MOUSE WHEEL"));
		addTexture("icon-result");
		addString(tr("Zoom map"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("ARROWS"));
		addTexture("icon-result");
		addString(tr("Scroll map"), font);
		_currentY += yGap2;
	}
	// ----
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("LMB"));
		addTexture("icon-result");
		addString(tr("Place tile"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("MMB"));
		addTexture("icon-result");
		addString(tr("Select tile"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("RMB"));
		addTexture("icon-result");
		addString(tr("Remove tile"), font);
		_currentY += yGap2;
	}
	// ----
	{
		const AutoBorder b(this, _currentY);
		addKey("s");
		addTexture("icon-result");
		addString(tr("Save map"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("z");
		addTexture("icon-result");
		addString(tr("Undo"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("y");
		addTexture("icon-result");
		addString(tr("Redo"), font);
		_currentY += yGap2;
	}
	// ----
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("ALT"));
		addTexture("icon-plus");
		addKey("f");
		addTexture("icon-result");
		addString(tr("Zoom out to fit"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey("f");
		addTexture("icon-result");
		addString(tr("Zoom out"), font);
		_currentY += yGap;
	}
	// --------

	_currentX = middle;
	_currentY = top;

	{
		const AutoBorder b(this, _currentY);
		addKey("+");
		addTexture("icon-result");
		addString(tr("Increase map height"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey("-");
		addTexture("icon-result");
		addString(tr("Decrease map height"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey("+");
		addTexture("icon-result");
		addString(tr("Increase map width"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey("-");
		addTexture("icon-result");
		addString(tr("Decrease map width"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey(tr("ARROWS"));
		addTexture("icon-result");
		addString(tr("Shift map"), font);
		_currentY += yGap2;
	}
	// ----
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("SPACE"));
		addTexture("icon-result");
		addString(tr("Rotate entity"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("ALT"));
		addKey(tr("SHIFT"));
		addTexture("icon-plus");
		addKey(tr("Mouse move"));
		addTexture("icon-result");
		addString(tr("Shift tile"), font);
		_currentY += yGap2;
	}
	// ----
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("d");
		addTexture("icon-result");
		addString(tr("Convert ice theme to desert"), font);
		_currentY += yGap;
	}
	{
		const AutoBorder b(this, _currentY);
		addKey(tr("CTRL"));
		addTexture("icon-plus");
		addKey("j");
		addTexture("icon-result");
		addString(tr("Convert rock theme to jungle"), font);
		_currentY += yGap;
	}
	// --------

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIMapEditorHelpWindow::~UIMapEditorHelpWindow ()
{
}

}
