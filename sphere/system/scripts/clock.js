// Game clock object by WIP
// This is a function that will create a game clock object.
// Just use the function clock.getTime() and it will
// return an object with seconds, minutes and hours
// Note: You must start the clock before it will give
// the correct time

function Clock()
{
	this.started = 0;
	this.seconds = 0;
	this.minutes = 0;
	this.hours = 0;
}

// Example: var GameClock = new Clock();

Clock.prototype.start = function()
{
	this.started = GetTime();
}

// Example: GameClock.start();

Clock.prototype.getTime = function()
{
	var current = GetTime() - this.started;
	var timeo = new Object();
	timeo.seconds = Math.floor((current)/1000 + this.seconds) % 60;
	timeo.minutes = Math.floor((current)/1000/60 + this.minutes) % 60;
	timeo.hours = Math.floor((current)/1000/60/60 + this.hours) % 60;
	return timeo;
}

// Example:
// var time = GameClock.getTime();
// font.drawText(10, 10, time.seconds);
//
// Every time you want to start keeping track of time, 
// start the clock. If you are loading a game, you should
// load the time into the clock object. That way, it can keep
// correct time with a loaded game.