function Particle(x,y,size,speed,dest,color)
{
	this.x = x
	this.y = y
	this.size = size
	this.speed = speed
	this.dest = dest
	this.current_dest = 0
	this.color = color
}

function Dest(x,y)
{
	this.x = x
	this.y = y
}

function RenderParticles()
{
    for (var i = 0; i < particles.length; ++i) {
        var p = particles[i];
	    particleimg.blitMask(p.x, p.y, p.color)
	}
}

function ProcParticles()
{
    for (var i = 0; i < particles.length; ++i) {
        var p = particles[i];
	    if (p.x > p.dest[p.current_dest].x) { p.x = p.x-p.speed }
	    if (p.x < p.dest[p.current_dest].x) { p.x = p.x+p.speed }
	
	    if (p.y > p.dest[p.current_dest].y) { p.y = p.y-p.speed }
	    if (p.y < p.dest[p.current_dest].y) { p.y = p.y+p.speed }
	}
}

function ChangeParticlesColor(color)
{
var count = 0
while (count < particles.length)
	{
	particles[count].color = color
	count++;
	}
}

function ChangeParticlesSpeed(speed)
{
var count = 0
while (count < particles.length)
	{
	particles[count].speed = speed
	count++;
	}
}

function ScaterParticles()
{
var count = 0
while (count < particles.length)
	{
	particles[count].dest[0] = new Dest(Random(-50,340),Random(-50,260)) 
	particles[count].dest[1] = new Dest(Random(-50,340),Random(-50,260)) 
	count++;
	}
}

function ScaterParticles2()
{
var count = 0
while (count < particles.length)
	{
	rtype = new Array()
		rtype[0] = Random(-340,-20)
		rtype[1] = Random(-240,-20)
		rtype[2] = Random(330,360)
		rtype[3] = Random(250,270)
	particles[count].dest[0] = new Dest(rtype[Random(0,3)],rtype[Random(0,3)]) 
	particles[count].dest[1] = new Dest(rtype[Random(0,3)],rtype[Random(0,3)]) 
	count++;
	}
}