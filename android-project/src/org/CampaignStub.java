package org;

import java.util.List;

public class CampaignStub {
	public CampaignStub(int id, List<CampaignMapStub> maps) {
		this.id = id;
		this.maps = maps;
	}

	int id;
	List<CampaignMapStub> maps;
}
