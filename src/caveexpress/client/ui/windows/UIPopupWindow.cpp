#include "UIPopupWindow.h"
#include "caveexpress/client/ui/nodes/UINodePopupBackground.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"

class UINodePopupButton: public UINodeButton {
public:
	UINodePopupButton(IFrontend *frontend, const std::string& title, const std::string& font = LARGE_FONT, const Color& color = colorWhite) :
			UINodeButton(frontend, title) {
		setFont(getFont(font), color);
		setTitleAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
		autoSize();
	}

	void addFocus (int32_t x, int32_t y) override
	{
		UINodeButton::addFocus(x, y);
		setBackgroundColor(colorGrayAlpha);
	}

	void removeFocus ()
	{
		UINodeButton::removeFocus();
		setBackgroundColor(colorNull);
	}

	virtual ~UINodePopupButton() {
	}
};

UIPopupWindow::UIPopupWindow (IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback) :
		UIWindow("popup", frontend, WINDOW_FLAG_MODAL)
{
	UINodePopupBackground *background = new UINodePopupBackground(frontend);
	add(background);

	UINodeLabel *label = new UINodeLabel(frontend, text);
	label->setFont(LARGE_FONT);
	label->setColor(colorWhite);
	label->alignTo(background, NODE_ALIGN_TOP | NODE_ALIGN_CENTER, 0.01f);
	if (background->getWidth() < label->getWidth()) {
		background->setSize(label->getWidth(), background->getHeight());
		background->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	}
	add(label);

	if (flags & UIPOPUP_OK) {
		UINodePopupButton *ok = new UINodePopupButton(frontend, tr("OK"));
		ok->addListener(UINodeListenerPtr(new UINodePopupListener(callback, UIPOPUP_OK)));
		ok->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, 0.01f);
		add(ok);
	}

	if (flags & UIPOPUP_CANCEL) {
		UINodePopupButton *cancel = new UINodePopupButton(frontend, tr("Cancel"));
		cancel->addListener(UINodeListenerPtr(new UINodePopupListener(callback, UIPOPUP_CANCEL)));
		cancel->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, 0.01f);
		add(cancel);
	}
}

bool UIPopupWindow::shouldDelete() const
{
	return true;
}
