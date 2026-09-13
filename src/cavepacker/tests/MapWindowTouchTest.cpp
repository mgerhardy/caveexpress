#include "tests/TestShared.h"
#ifdef scroll
#undef scroll
#endif
#include "ui/UI.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINode.h"
#include "client/Camera.h"
#include "client/ClientMap.h"
#include "common/MapSettings.h"
#include <SDL.h>

namespace cavepacker {

class MapWindowTouchTest: public AbstractTest {
protected:
	IUIMapWindow* _window = nullptr;
	IUINodeMap* _nodeMap = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		UI::get().init(_serviceProvider, _eventHandler, _testFrontend);
		_window = static_cast<IUIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		ASSERT_NE(nullptr, _window);
		_nodeMap = static_cast<IUINodeMap*>(_window->getNode(UINODE_MAP));
		ASSERT_NE(nullptr, _nodeMap);
		ClientMap& map = _nodeMap->getMap();
		ASSERT_TRUE(map.load("touch-pan-test", "Touch"));
		map.setSetting(msn::WIDTH, "20");
		map.setSetting(msn::HEIGHT, "15");
		map.start();
		_window->start();
	}

	void TearDown () override
	{
		_window = nullptr;
		_nodeMap = nullptr;
		UI::get().shutdown();
		AbstractTest::TearDown();
	}

	int centerX () const
	{
		return _testFrontend.getWidth() / 2;
	}

	int centerY () const
	{
		return _testFrontend.getHeight() / 2;
	}
};

TEST_F(MapWindowTouchTest, fingerDragPansMap)
{
	ClientMap& map = _nodeMap->getMap();
	const int before = map.getCamera().scrollOffsetX();
	ASSERT_TRUE(_window->onFingerMotion(1, centerX(), centerY(), 40, 0));
	EXPECT_EQ(before + 40, map.getCamera().scrollOffsetX());
}

TEST_F(MapWindowTouchTest, fingerDragOnUndoDoesNotPan)
{
	UINode* undo = _window->getNode("undo");
	ASSERT_NE(nullptr, undo);
	ASSERT_TRUE(undo->isVisible());
	const int x = undo->getRenderCenterX();
	const int y = undo->getRenderCenterY();
	ClientMap& map = _nodeMap->getMap();
	const int before = map.getCamera().scrollOffsetX();
	ASSERT_TRUE(_window->onFingerMotion(1, x, y, 40, 0));
	EXPECT_EQ(before, map.getCamera().scrollOffsetX());
}

TEST_F(MapWindowTouchTest, pinchZoomsImmediately)
{
	ClientMap& map = _nodeMap->getMap();
	_window->onFingerPress(1, centerX(), centerY());
	const float before = map.getZoom();
	ASSERT_TRUE(_window->onMultiGesture(0.0f, 0.05f, 2));
	EXPECT_GT(map.getZoom(), before);
	const float afterIn = map.getZoom();
	ASSERT_TRUE(_window->onMultiGesture(0.0f, -0.05f, 2));
	EXPECT_LT(map.getZoom(), afterIn);
}

TEST_F(MapWindowTouchTest, mouseDragStillPans)
{
	ClientMap& map = _nodeMap->getMap();
	const int x = centerX();
	const int y = centerY();
	_window->onMouseButtonPress(x, y, SDL_BUTTON_MIDDLE);
	const int before = map.getCamera().scrollOffsetX();
	_window->onMouseMotion(x + 25, y, 25, 0);
	EXPECT_EQ(before + 25, map.getCamera().scrollOffsetX());
	_window->onMouseButtonRelease(x + 25, y, SDL_BUTTON_MIDDLE);
}

TEST_F(MapWindowTouchTest, mouseWheelStillZooms)
{
	ClientMap& map = _nodeMap->getMap();
	const float before = map.getZoom();
	_window->onMouseWheel(0, 1);
	EXPECT_GT(map.getZoom(), before);
	const float afterIn = map.getZoom();
	_window->onMouseWheel(0, -1);
	EXPECT_LT(map.getZoom(), afterIn);
}

}
