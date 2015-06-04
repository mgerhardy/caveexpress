#include "TestShared.h"
#include "ui/UI.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINode.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/layouts/UIVBoxLayout.h"

class UITest: public AbstractTest {
protected:
	virtual void SetUp() override {
		AbstractTest::SetUp();
		UI::get().init(_serviceProvider, _eventHandler, _testFrontend);
	}
	virtual void TearDown() override {
		UI::get().shutdown();
		AbstractTest::TearDown();
	}
};

class TestNode: public UINode {
protected:
	bool _active;
public:
	TestNode(IFrontend* frontend, bool active) :
			UINode(frontend, "testnode"), _active(active) {
	}

	bool isActive() const override {
		if (!_active)
			return UINode::isActive();
		return true;
	}
};

class TestWindow: public UIWindow {
public:
	TestWindow(IFrontend* frontend) :
			UIWindow("testwindow", frontend) {
	}
};

TEST_F(UITest, testSimpleFocus) {
	TestWindow window(&_testFrontend);
	UINode *node1 = new TestNode(&_testFrontend, true);
	UINode *node2 = new TestNode(&_testFrontend, true);
	UINode *node3 = new TestNode(&_testFrontend, true);
	UINode *node4 = new TestNode(&_testFrontend, false);
	window.add(node1);
	window.add(node2);
	window.add(node3);
	window.add(node4);

	ASSERT_TRUE(window.addFirstFocus());
	ASSERT_TRUE(node1->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.nextFocus());
	ASSERT_FALSE(node1->hasFocus());
	ASSERT_TRUE(node2->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.addLastFocus());
	ASSERT_FALSE(node1->hasFocus());
	ASSERT_FALSE(node2->hasFocus());
	ASSERT_TRUE(node3->hasFocus());
	ASSERT_TRUE(window.hasFocus());
}

TEST_F(UITest, testPanelFocus) {
	TestWindow window(&_testFrontend);
	UINode *panel = new UINode(&_testFrontend, "panel");
	UINode *node1 = new TestNode(&_testFrontend, true);
	UINode *node2 = new TestNode(&_testFrontend, true);
	UINode *node3 = new TestNode(&_testFrontend, true);
	UINode *node4 = new TestNode(&_testFrontend, false);
	panel->add(node1);
	panel->add(node2);
	panel->add(node3);
	panel->add(node4);
	window.add(panel);

	ASSERT_TRUE(window.addFirstFocus());
	ASSERT_TRUE(panel->hasFocus());
	ASSERT_TRUE(node1->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.nextFocus());
	ASSERT_FALSE(node1->hasFocus());
	ASSERT_TRUE(node2->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.addLastFocus());
	ASSERT_FALSE(node1->hasFocus());
	ASSERT_FALSE(node2->hasFocus());
	ASSERT_TRUE(node3->hasFocus());
	ASSERT_TRUE(window.hasFocus());
}

TEST_F(UITest, testPanelWithOthersFocus) {
	TestWindow window(&_testFrontend);
	UINode *panel = new UINode(&_testFrontend, "panel");
	UINode *node1 = new TestNode(&_testFrontend, true);
	UINode *node2 = new TestNode(&_testFrontend, true);
	UINode *node3 = new TestNode(&_testFrontend, true);
	UINode *node4 = new TestNode(&_testFrontend, false);
	UINode *noPanelNode1 = new TestNode(&_testFrontend, true);
	UINode *noPanelNode2 = new TestNode(&_testFrontend, false);
	panel->add(node1);
	panel->add(node2);
	panel->add(node3);
	panel->add(node4);
	window.add(noPanelNode1);
	window.add(noPanelNode2);
	window.add(panel);

	ASSERT_TRUE(panel->isActive());

	ASSERT_TRUE(window.addFirstFocus());
	ASSERT_TRUE(noPanelNode1->hasFocus());
	ASSERT_FALSE(noPanelNode2->hasFocus());
	ASSERT_FALSE(panel->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.nextFocus());
	ASSERT_TRUE(panel->hasFocus());
	ASSERT_FALSE(noPanelNode1->hasFocus());
	ASSERT_FALSE(noPanelNode2->hasFocus());
	ASSERT_TRUE(node1->hasFocus());
	ASSERT_TRUE(window.hasFocus());

	ASSERT_TRUE(window.addLastFocus());
	ASSERT_FALSE(node1->hasFocus());
	ASSERT_FALSE(node2->hasFocus());
	ASSERT_TRUE(node3->hasFocus());
	ASSERT_TRUE(window.hasFocus());
}

TEST_F(UITest, testVisibleFocus) {
	TestWindow window(&_testFrontend);
	UINode *node1 = new TestNode(&_testFrontend, true);
	window.add(node1);
	ASSERT_TRUE(window.addFirstFocus());
	ASSERT_TRUE(node1->hasFocus());
	node1->setVisible(false);
	ASSERT_FALSE(node1->hasFocus());
}

TEST_F(UITest, testAlignTo) {
	TestWindow window(&_testFrontend);
	UINode *node1 = new TestNode(&_testFrontend, false);
	node1->setSize(1.0f, 0.5f);
	window.add(node1);

	UINode *node2 = new TestNode(&_testFrontend, false);
	node2->setSize(0.1f, 0.1f);
	node2->alignTo(node1, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, 0.0f);
	window.add(node2);

	ASSERT_DOUBLE_EQ(node1->getX(), node2->getX());
	ASSERT_DOUBLE_EQ(node1->getBottom(), node1->getHeight());
	ASSERT_DOUBLE_EQ(0.5f, node1->getBottom());
	ASSERT_DOUBLE_EQ(0.1f, node2->getHeight());
	ASSERT_DOUBLE_EQ(0.4f, node2->getY());
	const float expectedNode2Y = node1->getBottom() - node2->getHeight();
	ASSERT_DOUBLE_EQ(expectedNode2Y, node2->getY());
}

TEST_F(UITest, testHLayout) {
	TestWindow window(&_testFrontend);
	UINode *nodeH = new TestNode(&_testFrontend, false);
	nodeH->setSize(1.0f, 1.0f);
	nodeH->setLayout(new UIHBoxLayout(0.0f, false));
	UINode *node1 = new TestNode(&_testFrontend, false);
	node1->setSize(0.1f, 0.1f);
	UINode *node2 = new TestNode(&_testFrontend, false);
	node2->setSize(0.1f, 0.1f);
	nodeH->add(node1);
	nodeH->add(node2);
	window.add(nodeH);
	ASSERT_DOUBLE_EQ(0.0f, node1->getX());
	ASSERT_DOUBLE_EQ(0.0f, node1->getY());
	ASSERT_DOUBLE_EQ(0.1f, node2->getX());
	ASSERT_DOUBLE_EQ(0.0f, node2->getY());
}

TEST_F(UITest, testVLayout) {
	TestWindow window(&_testFrontend);
	UINode *nodeV = new TestNode(&_testFrontend, false);
	nodeV->setSize(1.0f, 1.0f);
	nodeV->setLayout(new UIVBoxLayout(0.0f, false));
	UINode *node1 = new TestNode(&_testFrontend, false);
	node1->setSize(0.1f, 0.1f);
	UINode *node2 = new TestNode(&_testFrontend, false);
	node2->setSize(0.1f, 0.1f);
	nodeV->add(node1);
	nodeV->add(node2);
	window.add(nodeV);
	ASSERT_DOUBLE_EQ(0.0f, node1->getX());
	ASSERT_DOUBLE_EQ(0.0f, node1->getY());
	ASSERT_DOUBLE_EQ(0.0f, node2->getX());
	ASSERT_DOUBLE_EQ(0.1f, node2->getY());
}

TEST_F(UITest, testRestart) {
	UI::get().initRestart();
	UI::get().restart();
}
