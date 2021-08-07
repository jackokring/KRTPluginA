#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	p->addModel(modelA);
	p->addModel(modelMu);
	p->addModel(modelT);
	p->addModel(modelL);
	p->addModel(modelD);
	p->addModel(modelR);

	p->addModel(modelOm);
	p->addModel(modelV);
	p->addModel(modelF);
	p->addModel(modelW);
	p->addModel(modelY);
	
}
