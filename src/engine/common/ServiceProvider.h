#pragma once

#include <string>

// forward decl
class CampaignManager;
class MapManager;
class IGameStatePersister;
class INetwork;
class TextureDefinition;
class IFrontend;

class ServiceProvider {
private:
	MapManager* _mapManager;
	INetwork* _network;
	INetwork* _loopback;
	TextureDefinition* _textureDefinition;
	INetwork* _currentNetwork;

public:
	ServiceProvider ();
	virtual ~ServiceProvider ();

	void initTextureDefinition (IFrontend *frontend, const std::string& textureSize);
	void init (IFrontend *frontend);

	// switches the network to either loopback or to real network
	void updateNetwork (bool network);
	// get the current network implementation - can either be loopback or real network
	// note that this value should not get cached somewhere, as the implementation behind
	// this might change during runtime (switching between loopback and real network)
	INetwork& getNetwork ();
	MapManager& getMapManager ();
	// get the texture definition that depends on your current resolution. Either the 'big'
	// or the 'small' definition was used.
	TextureDefinition& getTextureDefinition ();
};

inline INetwork& ServiceProvider::getNetwork ()
{
	return *_currentNetwork;
}

inline MapManager& ServiceProvider::getMapManager ()
{
	return *_mapManager;
}

inline TextureDefinition& ServiceProvider::getTextureDefinition ()
{
	return *_textureDefinition;
}
