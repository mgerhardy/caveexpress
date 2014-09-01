#pragma once

// forward decl
class INetwork;
class IGameStatePersister;
class INetwork;
class TextureDefinition;
class IFrontend;

class ServiceProvider {
private:
	IGameStatePersister* _persister;
	INetwork* _network;
	TextureDefinition* _textureDefinition;

public:
	ServiceProvider ();
	virtual ~ServiceProvider ();

	void init (IFrontend *frontend);

	IGameStatePersister& getGameStatePersister ();
	INetwork& getNetwork ();
	TextureDefinition& getTextureDefinition ();
};

inline INetwork& ServiceProvider::getNetwork ()
{
	return *_network;
}

inline IGameStatePersister& ServiceProvider::getGameStatePersister ()
{
	return *_persister;
}

inline TextureDefinition& ServiceProvider::getTextureDefinition ()
{
	return *_textureDefinition;
}
