//circles.js by Eric Duvic (geno023@cox.net)

//all functions return an image that can possibly be stored in another variable for later use
//used to return surfaces until i found out its much much faster to blit an image than a surface
//Special thanks goes out to flik(for almost everything :D), malis(for various algorithms :D), and beaker(for his info on how to speed things up)

var rdnfactor = ( Math.PI /180);
var sin = Math.sin;
var cos = Math.cos;
var round = Math.round;

function LineCircle( radius, color )
{
  var circlepic = CreateSurface( ( ( radius * 2 ) + 1 ), ( ( radius * 2 ) + 1 ), CreateColor(0,0,0,0) );
  
  var newcoordX = ( radius + round( radius * cos( 0 * rdnfactor ) ) );
  var newcoordY = ( radius + round( radius * sin( 0 * rdnfactor ) ) );
  var oldcoordX, oldcoordY;
  
  for ( i = 0; i <= 360; i+=1 )
  {
  
    oldcoordX = newcoordX;
    oldcoordY = newcoordY;
    
    newcoordX = ( radius + round( radius * cos( i * rdnfactor ) ) );
    newcoordY = ( radius + round( radius * sin( i * rdnfactor ) ) );
    
    circlepic.line(
										oldcoordX,
										oldcoordY,
										newcoordX,
										newcoordY,
										color
										);
  
		//circlepic.setPixel( ( radius + round( radius * cos( i * rdnfactor ) ) ), ( radius + round( radius * sin( i * rdnfactor ) ) ), color );
  }
  return circlepic.createImage();
}

function Circle( radius, color )
{
  return GradientCircle( radius, color, color );
}

function GradientCircle( radius, color1, color2 )
{
  var r, r2, g, g2, b, b2, a, a2;
  var r_r, g_g, b_b, a_a;
  
  var buffer =  CreateSurface(  ( radius * 2 ) , ( radius * 2 ), CreateColor(0,0,0,0) );
  
  r = color1.red;
  r2 = color2.red;
  g = color1.green;
  g2 = color2.green;
  b = color1.blue;
  b2 = color2.blue;
  a = color1.alpha;
  a2 = color2.alpha;
  
  r_r = (r2 - r) / radius;
  g_g = (g2 - g) / radius;
  b_b = (b2 - b) / radius;
  a_a = (a2 - a) / radius;
  
  buffer.setBlendMode( REPLACE );
  
  for ( rd = radius; rd > 0; rd-- )
  {
    
    color = CreateColor( r + ( r_r * rd ), g + ( g_g * rd ), b + ( b_b * rd ), a + ( a_a * rd ) );
    
    for ( ang = 0; ang <= 270; ang++ )
    {
      
      buffer.line(
									( round( buffer.width / 2 ) + round( rd * cos( ang * rdnfactor ) ) ),
									( round( buffer.height / 2 ) + round( rd * sin( ang * rdnfactor ) ) ),
									( round( buffer.width / 2 ) + round( rd * cos( ang * rdnfactor ) ) ),
									( round( buffer.height / 2 ) - round( rd * sin( ang * rdnfactor ) ) ),
									color
									);
			buffer.line(
									( round( buffer.width / 2 ) + round( rd * cos( ang * rdnfactor ) ) ),
									( round( buffer.height / 2 ) + round( rd * sin( ang * rdnfactor ) ) ),
									( round( buffer.width / 2 ) - round( rd * cos( ang * rdnfactor ) ) ),
									( round( buffer.height / 2 ) + round( rd * sin( ang * rdnfactor ) ) ),
									color
									);
      
    }
    
  }
  
  return buffer.createImage();
}