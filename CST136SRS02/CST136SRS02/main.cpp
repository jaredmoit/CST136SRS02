#include "stdafx.h"
#include <vector>
#include <cstdlib>
#include <typeinfo>
#include <exception>
#include "Raft.h"
#include "Wind.h"
#include "Water.h"
#include "Canoe.h"
#include "Sailboat.h"
#include <iostream>
#include <cassert>

void simulateJourney(std::vector<Boat*> boats, const int targetLat, const int targetLng)
{
	//Wind and water apply to all boats are change slightly ever time through
	static Wind wind = Wind();
	static Water water = Water();

	static const int MAX_INT = std::numeric_limits<int>::max();
	int activeBoats = boats.size();
	while (activeBoats > 0) {
		for (Boat* boat : boats)
		{
			
			//FOR DEMONSTRATION PURPOSES ONLY, I GET THE BOAT NAME
			boat->getName();
			
			//if boat is not capsized or completed, continue
			if (!boat->isCapsized() && !boat->isJourneyComplete())
			{
				//capsize boat if waves are too high
				if (water.getWaveSize() > boat->getHull()->getMaxWaveHeight())
				{
					boat->capsizeBoat();
					activeBoats--;
					break;
				}
				assert(!boat->isCapsized());

				//move towards target
				int newLat = boat->getChart().getLat();
				int newLng = boat->getChart().getLng();
				int newAngle = boat->getChart().getAngle();

				//have wind effect boat
				switch (wind.getDirection())
				{
				case Wind::Direction::North:
					newLng += wind.getMPH();
					break;
				case Wind::Direction::East:
					newLat -= wind.getMPH();
					break;
				case Wind::Direction::South:
					newLng -= wind.getMPH();
					break;
				case Wind::Direction::West:
					newLat += wind.getMPH();
					break;
				default:
					break;
				}

				//move towards target
				if (boat->getPropulsion() != nullptr) {
					
					//calc angle to target
					newAngle = (atan2(targetLng - newLng, targetLat - newLat) * 180) / 3.14;
					if(abs(newAngle - boat->getChart().getAngle()) > boat->getHull()->getMinTurnRadius())
					{
						//angle changed too quickly, slow down
						
						if(newAngle >= boat->getChart().getAngle())
						{
							//turn up
							newAngle = boat->getChart().getAngle(); // reset angle
							newAngle += boat->getHull()->getMinTurnRadius();
						}
						else
						{
							//turn down
							newAngle = boat->getChart().getAngle(); // reset angle
							newAngle -= boat->getHull()->getMinTurnRadius();
						}
					}

					//if sailboat, increase speed dependent on wind
					float sailBonus = 0; 
					if(typeid(boat) == typeid(Sailboat))
					{
						sailBonus = wind.getMPH() * 0.1;
					}
					
					//move boat towards target
					newLat += cos(newAngle) * (boat->getPropulsion()->getKnots() + sailBonus);
					newLng += sin(newAngle) * (boat->getPropulsion()->getKnots() + sailBonus);
				}
				
				//update chart
				boat->getChart().setCoordinates(newLat, newLng, newAngle);       

				//update time elapsed
				boat->getChart().incrementTimeElapsed();
				if (boat->getChart().getTimeElapsed() >= MAX_INT) throw std::overflow_error("Time overflow.");

				//test if boat is near target
				if (abs(boat->getChart().getLat() - targetLat) <= 10
					&& abs(boat->getChart().getLng() - targetLng) <= 10)
				{
					//JOURNEY COMPLETE!
					boat->completeJourney();
					activeBoats--;
				}
			}
		}

		//end of the loop / new hour
		wind.update();
		water.update();
	}

	//delete raw pointers
	for(Boat * boat: boats)
	{
		if (boat->getHull() != nullptr) delete boat->getHull();
		if (boat->getPropulsion() != nullptr) delete boat->getPropulsion();
	}

	//simulation complete
}

int main()
{
	// vector of 3 different boats
	std::vector<Boat*> boats(0);
	Raft raft = Raft("No Regerts");
	Canoe canoe = Canoe("VolCanoe");
	Sailboat sailboat = Sailboat("Breakin' Wind");
	boats.push_back(&raft);
	boats.push_back(&canoe);
	boats.push_back(&sailboat);

	//boats start at 0,0
	//ends at 1000, 1000
	const int targetLat = 1000;
	const int targetLng = 1000;

	try
	{
		simulateJourney(boats, targetLat, targetLng);
	}
	catch(const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

