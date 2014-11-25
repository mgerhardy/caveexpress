package org;

public class CampaignMapStub {
	public CampaignMapStub(String id, String name, boolean locked, boolean initialLockedState, int time,
			int finishPoints, byte stars) {
		this.id = id;
		this.name = name;
		this.locked = locked;
		this.initialLockedState = initialLockedState;
		this.time = time;
		this.finishPoints = finishPoints;
		this.stars = stars;
	}

	String id;
	String name;
	boolean locked;
	boolean initialLockedState;
	int time;
	int finishPoints;
	byte stars;
}
