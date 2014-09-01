#include "UINodeMap.h"
#include "client/ui/UI.h"
#include "client/ui/BitmapFont.h"
#include "client/ui/nodes/UINodeBar.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "client/network/SoundHandler.h"
#include "client/network/InitDoneHandler.h"
#include "client/round/RoundController.h"
#include "shared/network/ProtocolHandlerRegistry.h"

UINodeMap::UINodeMap (IFrontend *frontend, RoundController& controller) :
		UINode(frontend), _controller(controller)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_INITDONE, new InitDoneHandler());
	r.registerClientHandler(protocol::PROTO_SOUND, new SoundHandler());

	setBackgroundColor(colorNull);

	_font = getFont();
	_resultPanelLeft = loadTexture("result-panel-left");
	_resultPanelRight = loadTexture("result-panel-right");
}

UINodeMap::~UINodeMap ()
{
}

bool UINodeMap::onPush ()
{
	const bool ret = UINode::onPush();
	_controller.handleStateChange(STATE_PREPARE);
	return ret;
}

void UINodeMap::update (uint32_t deltaTime)
{
	UINode::update(deltaTime);
	_controller.update(deltaTime);
}

void UINodeMap::renderFailed (int x, int y) const
{
	const Color overlayColor = { 0.0f, 0.0f, 0.0f, 0.5f };
	_frontend->renderFilledRect(0, 0, _frontend->getWidth(), _frontend->getHeight(), overlayColor);
	const std::string text = "Time's up!";
	const int textWidth = _font->getTextWidth(text);
	const int textHeight = _font->getCharHeight();
	const int textX = x + getRenderCenterX() - textWidth / 2;
	const int textY = y + getRenderCenterY() - textHeight / 2;
	const float relTextX = textX / static_cast<float>(_frontend->getWidth());
	const float relTextY = textY / static_cast<float>(_frontend->getHeight());
	const float relTextWidth = textWidth / static_cast<float>(_frontend->getWidth());
	const float relTextHeight = textHeight / static_cast<float>(_frontend->getHeight());
	const int boxX = (relTextX - relTextWidth) * _frontend->getWidth();
	const int boxY = (relTextY - 2.0f * relTextHeight) * _frontend->getHeight();
	const int boxWidth = (relTextWidth * 3.0f) * _frontend->getWidth();
	const int boxHeight = (relTextHeight * 5.0f) * _frontend->getHeight();
	// TODO: use nice graphics here
	_frontend->renderFilledRect(boxX, boxY, boxWidth, boxHeight, colorRed);
	_font->print(text, colorWhite, textX, textY);
}

void UINodeMap::renderResult (int x, int y) const
{
	const int resultWidth = getRenderWidth() / 3;
	const int resultHeight = getRenderHeight() / 3;
	TexturePtr resultPanel = _resultPanelRight;
	int resultPanelX = _controller.getLocationScreenX();
	const int resultPanelY = _controller.getLocationScreenY() - resultPanel->getHeight() / 10;
	int nameX = resultPanelX + resultWidth / 5.5;
	const int nameY = resultPanelY + resultHeight / 10;
	if (resultPanelX > getRenderWidth() / 2) {
		resultPanel = _resultPanelLeft;
		resultPanelX = resultPanelX - resultWidth;
		nameX = resultPanelX + resultWidth / 20;
	}
//	@todo make an upside down panel if the dot is too low
//	if (resultPanelY > getRenderWidth() - resultHeight) {
//		resultY = getRenderWidth() - resultHeight - marker->getHeight();
//	}
	_frontend->renderImage(resultPanel.get(), resultPanelX, resultPanelY, resultWidth, resultHeight, 0, 1.0f);
	const Location& l = _controller.getCurrentLocation();
	// TODO: use nice graphics here and remove the hardcoded hacks
	_font->print(l.location, colorBlack, nameX, nameY);
	_font->print("TODO, Add Full, Location", colorBlack, nameX, nameY + 20);
	_font->print("Photo by @todo", colorBlack, nameX, nameY + 40);
	_font->print("SCORE", colorBlack, nameX, nameY + resultHeight - 40);
	_font->print(string::toString(l.getPoints()), colorBlack, nameX, nameY + resultHeight - 70);
	_font->print("DISTANCE", colorBlack, nameX + resultWidth / 2, nameY + resultHeight - 40);
	// @todo convert distance to km
	_font->print(string::toString(l.getDistance()) + "km", colorBlack, nameX + resultWidth / 2, nameY + resultHeight - 70);
//	@todo Should we show time?
//	_whiteFont->print(string::toString(l.getTimeTaken() / 1000.0) + "s", colorBlack, nameX + resultWidth / 2, nameY + resultHeight - 80);
}

void UINodeMap::renderMarkers (int x, int y) const
{
	const int width = 10;
	const int height = width;
	const int clickedDotX = x + _controller.getClickedScreenX() - width / 2;
	const int clickedDotY = y + _controller.getClickedScreenY() - height / 2;
	// TODO: use nice graphics here
	_frontend->renderFilledRect(clickedDotX, clickedDotY, width, height, colorRed);
}

const TexturePtr& UINodeMap::setImage (const std::string& texture)
{
	const TexturePtr& image = UINode::setImage(texture);
	if (!hasImage())
		return image;

	float aspectRatio = image->getWidth() / static_cast<float>(image->getHeight());
	if (aspectRatio >= 1.0f) {
		setSize(1.0f, ((_frontend->getWidth() / static_cast<float>(image->getWidth())) * image->getHeight()) / _frontend->getHeight());
	} else {
		setSize(aspectRatio * (static_cast<float>(_frontend->getHeight()) / _frontend->getWidth()), 1.0f);
	}
	setAlignment(NODE_ALIGN_MIDDLE|NODE_ALIGN_CENTER);

	return image;
}

void UINodeMap::render (int x, int y) const
{
	UINode::render(x, y);
	switch (_controller.getState()) {
	case STATE_CLICKED:
		renderMarkers(x, y);
		renderResult(x, y);
		break;
	case STATE_FAILED:
		renderFailed(x, y);
		break;
	default:
		break;
	}

	const bool debug = Config.getConfigVar("debugui")->getBoolValue();
	if (!debug)
		return;

	_font->print("State: " + string::toString(_controller.getState()), colorWhite, 100, 30);
}

void UINodeMap::handleDrop (uint16_t x, uint16_t y)
{
	UINode::handleDrop(x, y);
	_controller.handleClick(x, y);

	if (_controller.getState() == STATE_ROUNDEND) {
		UI::get().pop();
		UI::get().push(UI_WINDOW_ROUNDRESULT);
	}
}

bool UINodeMap::wantFocus () const
{
	return _controller.isClickAllowed();
}
