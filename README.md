"# RedBirdCenter - an Edition of David Churhills CommandCenter SC2 bot 

[UML Diagram](https://imgur.com/a/qG4B2iP)

Currently structured similarly to the original bot with most noteworthy changes and additions to
*StrategyManager.cpp/H
  *Filled out methods to read from and execute a build order goal, a trigger for the bot to attack based on unit count. Currently only works for Terran, similar code for other two races and needs paramaters/input for what units track
*ProductionManager.cpp
  * ManageBuildOrderQueue(), fixBuildOrderDeadlock(); Gets the bot to recognize impending or current supply blocks and queue more to be built
  * TODO Idea for onUnitDestroy(), a trigger for changes in the build order/macroing if something "important" dies
*Timed Attacks
	* CombatCommander.cpp : idea for cyclical attacks or in-game-time based attacks, should probably be turned into it's own method(s) and global variables so it can be tied to variable input from the config file
	
	


