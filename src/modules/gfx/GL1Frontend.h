#pragma once

#include "AbstractGLFrontend.h"

class GL1Frontend: public AbstractGLFrontend {
protected:
	void setColorPointer (const Color& color, int amount);
public:
	explicit GL1Frontend (std::shared_ptr<IConsole> console);
	virtual ~GL1Frontend ();

	void initRenderer () override;
	void renderBatches () override;
	void updateViewport (int x, int y, int width, int height) override;
};
