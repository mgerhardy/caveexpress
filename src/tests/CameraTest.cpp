#include <gtest/gtest.h>
#include "client/Camera.h"

class CameraTest: public ::testing::Test {
};

TEST_F(CameraTest, smallMapIsCentered)
{
	Camera cam;
	cam.init(800, 600, 5, 5, 64, 1.0f);
	EXPECT_EQ((800 - 320) / 2, cam.getViewportX());
	EXPECT_EQ((600 - 320) / 2, cam.getViewportY());
}

TEST_F(CameraTest, playerCenteredOnLargeMap)
{
	Camera cam;
	cam.init(800, 600, 50, 40, 64, 1.0f);
	cam.update(vec2(20.0f, 15.0f), 0, 1.0f);
	EXPECT_EQ(400 - 20 * 64, cam.getViewportX());
	EXPECT_EQ(300 - 15 * 64, cam.getViewportY());
}

TEST_F(CameraTest, zoomOutCentersPlayer)
{
	Camera cam;
	const float zoom = 0.5f;
	cam.init(800, 600, 50, 40, 64, zoom);
	const float px = 20.0f;
	const float py = 15.0f;
	cam.update(vec2(px, py), 0, zoom);
	const int playerScreenX = cam.getViewportX() + static_cast<int>(px * 64.0f * zoom);
	const int playerScreenY = cam.getViewportY() + static_cast<int>(py * 64.0f * zoom);
	EXPECT_NEAR(playerScreenX, 400, 1);
	EXPECT_NEAR(playerScreenY, 300, 1);
}

TEST_F(CameraTest, zoomOutAtRightEdgeDoesNotShowBlack)
{
	Camera cam;
	const int viewW = 800;
	const int viewH = 600;
	const int tilesX = 50;
	const int tilesY = 40;
	const int scale = 64;
	const float zoom = 0.5f;
	cam.init(viewW, viewH, tilesX, tilesY, scale, zoom);
	cam.update(vec2(49.0f, 20.0f), 0, zoom);
	const int mapW = static_cast<int>(static_cast<float>(tilesX * scale) * zoom);
	const int mapH = static_cast<int>(static_cast<float>(tilesY * scale) * zoom);
	EXPECT_LE(cam.getViewportX(), 0);
	EXPECT_GE(cam.getViewportX() + mapW, viewW);
	EXPECT_LE(cam.getViewportY(), 0);
	EXPECT_GE(cam.getViewportY() + mapH, viewH);
}

TEST_F(CameraTest, zoomOutAtLeftEdgeStaysPinned)
{
	Camera cam;
	cam.init(800, 600, 50, 40, 64, 0.5f);
	cam.update(vec2(0.5f, 1.0f), 0, 0.5f);
	EXPECT_EQ(0, cam.getViewportX());
}

TEST_F(CameraTest, zoomedOutMapThatFitsIsCentered)
{
	Camera cam;
	cam.init(800, 600, 10, 8, 64, 0.5f);
	const int mapW = static_cast<int>(10 * 64 * 0.5f);
	const int mapH = static_cast<int>(8 * 64 * 0.5f);
	EXPECT_EQ((800 - mapW) / 2, cam.getViewportX());
	EXPECT_EQ((600 - mapH) / 2, cam.getViewportY());
}
