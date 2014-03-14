#include "ConsoleFrontend.h"
#include "engine/common/TextConsole.h"
#include "engine/common/Logger.h"
#include "engine/common/EventHandler.h"
#include "engine/common/CommandSystem.h"

ConsoleFrontend::ConsoleFrontend (TextConsole& console) :
		_eventHandler(nullptr), _console(console)
{
	Logger::get().addConsole(&_console);
}

ConsoleFrontend::~ConsoleFrontend ()
{
	Logger::get().removeConsole(&_console);
}

void ConsoleFrontend::update (uint32_t deltaTime)
{
	_console.update(deltaTime);
}

void ConsoleFrontend::onMapLoaded ()
{
}

void ConsoleFrontend::shutdown ()
{
	_eventHandler->removeObserver(&_console);
}

bool ConsoleFrontend::handlesInput () const
{
	return true;
}

void ConsoleFrontend::render ()
{
	_console.render();
}

void ConsoleFrontend::connect ()
{
}

int ConsoleFrontend::init (int width, int height, bool fullscreen, EventHandler &eventHandler)
{
	_eventHandler = &eventHandler;
	_eventHandler->registerObserver(&_console);

	_console.init(this);

	return 1;
}

void ConsoleFrontend::initUI (ServiceProvider& serviceProvider)
{
}
