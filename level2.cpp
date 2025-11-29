#include <iostream>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include<ctime>
#include<cstdlib>


using namespace sf;
using namespace std;

int screen_x = 1136;
int screen_y = 896;

void display_level(RenderWindow& window, char** lvl,
                   Texture& bgTex, Sprite& bgSprite,
                   Texture& blockTexture, Sprite& blockSprite,
                   Sprite& triLeftSprite,Sprite& triRightSprite,
                   const int height, const int width, const int cell_size)

{
	window.draw(bgSprite);

	for (int i = 0; i < height; i += 1)
	{
		for (int j = 0; j < width; j += 1)
		{

			if (lvl[i][j] == '#')
			{
				blockSprite.setPosition(j * cell_size, i * cell_size);
				window.draw(blockSprite);
			}
			else if (lvl[i][j] == '\\')   // IMPORTANT: double slash!
			{
			    triLeftSprite.setPosition(j * cell_size, i * cell_size);
			    window.draw(triLeftSprite);
			}
			else if(lvl[i][j]=='/')
			{
			    triRightSprite.setPosition(j * cell_size, i * cell_size);
			    window.draw(triRightSprite);
			}
		}
	}

}

void player_gravity(char** lvl, float& offset_y, float& velocityY, bool& onGround, const float& gravity, float& terminal_Velocity, float& player_x, float& player_y, const int cell_size, int& Pheight, int& Pwidth)
{
	offset_y = player_y;

	offset_y += velocityY;

	char bottom_left_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x ) / cell_size];
	char bottom_right_down = lvl[(int)(offset_y  + Pheight) / cell_size][(int)(player_x + Pwidth) / cell_size];
	char bottom_mid_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x + Pwidth / 2) / cell_size];

	if (bottom_left_down == '#' || bottom_mid_down == '#' || bottom_right_down == '#')
	{
		onGround = true;
	}
	else
	{
		player_y = offset_y;
		onGround = false;
	}

	if (!onGround)
	{
		velocityY += gravity;
		if (velocityY >= terminal_Velocity) velocityY = terminal_Velocity;
	}

	else
	{
		velocityY = 0;
	}
}


int main()
{

	RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Resize);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	//level specifics
	const int cell_size = 64;
	const int height = 14;
	const int width = 18;
	char** lvl;

	//level and background textures and sprites
	Texture bgTex;
	Sprite bgSprite;
	Texture blockTexture;
	Sprite blockSprite;

	bgTex.loadFromFile("bg2.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0,0);
	
	
// --- scale background to fill window ---
float scaleX = (float)screen_x / bgTex.getSize().x;
float scaleY = (float)screen_y / bgTex.getSize().y;
bgSprite.setScale(scaleX, scaleY);

	blockTexture.loadFromFile("block41.png");
	blockSprite.setTexture(blockTexture);
	Texture triLeftTex;

	Sprite triLeftSprite;


	triLeftTex.loadFromFile("blockt.png");


	triLeftSprite.setTexture(triLeftTex);
	Texture triRightTex;
	
	Sprite triRightSprite;


	triRightTex.loadFromFile("blockti.png");


	triRightSprite.setTexture(triRightTex);
	
// scale triangle to fill cell
triLeftSprite.setScale(
    (float)cell_size / triLeftTex.getSize().x,
    (float)cell_size / triLeftTex.getSize().y
);

//
triRightSprite.setScale(
    (float)cell_size / triRightTex.getSize().x,
    (float)cell_size / triRightTex.getSize().y
);
	

	//Music initialisation
	Music lvlMusic;

	lvlMusic.openFromFile("mus.ogg");
	lvlMusic.setVolume(20);
	lvlMusic.play();
	lvlMusic.setLoop(true);

	//player data
	float player_x = 500;
	float player_y = 150;

	float speed = 5;

	const float jumpStrength = -20; // Initial jump velocity
	const float gravity = 1;  // Gravity acceleration

	bool isJumping = false;  // Track if jumping

	bool up_collide = false;
	bool left_collide = false;
	bool right_collide = false;

	Texture PlayerTexture;
	Sprite PlayerSprite;

	bool onGround = false;

	float offset_x = 0;
	float offset_y = 0;
	float velocityY = 0;

	float terminal_Velocity = 20;

	int PlayerHeight = 102;
	int PlayerWidth = 96;

	bool up_button = false;

	char top_left = '\0';
	char top_right = '\0';
	char top_mid = '\0';

	char left_mid = '\0';
	char right_mid = '\0';

	char bottom_left = '\0';
	char bottom_right = '\0';
	char bottom_mid = '\0';

	char bottom_left_down = '\0';
	char bottom_right_down = '\0';
	char bottom_mid_down = '\0';

	char top_right_up = '\0';
	char top_mid_up = '\0';
	char top_left_up = '\0';

	PlayerTexture.loadFromFile("player.png");
	PlayerSprite.setTexture(PlayerTexture);
	PlayerSprite.setScale(3,3);
	PlayerSprite.setPosition(player_x, player_y);


	//creating level array
	lvl = new char* [height];
	for (int i = 0; i < height; i += 1)
	{
		lvl[i] = new char[width];
	}
	srand(time(0));
	int option;
	option=rand()%5;
	
	for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
        lvl[i][j] = ' ';
	
	switch(option)
	{
	
	case 0:
	{
	for(int i=0;i<width;i++)
	{	
		
		if(i==0||i==1)
			lvl[7][i]='#';
		if(i<=3)
			lvl[10][i]='#';
		
	}
	for(int i=0;i<width;i++)
	{	
		if(i>8&&i<16)
			lvl[2][i]='#';
			
		if(i>9&&i<12)
			lvl[5][i]='#';
			
		if(i>15)	
			lvl[5][i]='#';
		if(i>12&&i<17)
			
			lvl[8][i]='#';
		if(i>14)	
			lvl[11][i]='#';
	}		
		
	//Slant platforms
	for(int i=0;i<width;i++)
	{
		if(i<=2)
		{
		  lvl[3][i]='#';
		  lvl[3][3]='\\';
		}
		
		lvl[3][3]='#';
		lvl[3][4]='\\';
		
		lvl[4][4]='#';
		lvl[4][5]='\\';
		
		lvl[5][5]='#';
		lvl[5][6]='\\';
	
		lvl[6][6]='#';
		lvl[6][7]='\\';
		
		lvl[7][7]='#';
		lvl[7][8]='\\';
	
		lvl[8][8]='#';
		lvl[8][9]='\\';
	
		lvl[9][9]='#';
		lvl[9][10]='\\';
	
		lvl[10][10]='#';
		lvl[10][11]='\\';
	}	
	
	break;
	}
	
	case 1:
	{
	for(int i=0;i<width;i++)
	{
		if(i>=0&&i<=6)
			lvl[2][i]='#';
			
		if(i>0&&i<4)
			lvl[5][i]='#';
			
		if(i>5&&i<8)	
			lvl[5][i]='#';
		if(i>0&&i<6)
			
			lvl[8][i]='#';
		if(i<5)	
			lvl[11][i]='#';
	}
	for(int i=0;i<width;i++)
	{	
		
		if(i==17||i==16)
			lvl[7][i]='#';
		if(i>=14)
			lvl[10][i]='#';
		
	}
	//Slant platforms
	for(int i=0;i<width;i++)
	{
		if(i>=16)
		{
		  lvl[3][i]='#';
		  //lvl[3][15]='/';
		}
		
		lvl[3][15]='#';
		lvl[3][14]='/';
		
		lvl[4][14]='#';
		lvl[4][13]='/';
		
		lvl[5][13]='#';
		lvl[5][12]='/';
	
		lvl[6][12]='#';
		lvl[6][11]='/';
		
		lvl[7][11]='#';
		lvl[7][10]='/';
	
		lvl[8][10]='#';
		lvl[8][9]='/';
	
		lvl[9][9]='#';
		lvl[9][8]='/';
	
		lvl[10][8]='#';
		lvl[10][7]='/';
	}
	break;
	}
	
	case 2:
	{
		for(int i=0;i<width;i++)
		{
			if(i>=0&&i<=4)
				lvl[3][i]='#';
			if(i>=7&&i<=10)
				lvl[3][i]='#';
			if(i>=13)
				lvl[3][i]='#';
			if(i>=15)
				lvl[5][i]='#';
			if(i>=12)
				lvl[8][i]='#';
		}
		//Slant platforms
		for(int i=0;i<width;i++)
		{
			if(i<=2)
			{
			  lvl[6][i]='#';
			  lvl[6][3]='\\';
			}
			
			lvl[7][3]='#';
			lvl[7][4]='\\';
			
			lvl[8][4]='#';
			lvl[8][5]='\\';
			
			lvl[9][5]='#';
			lvl[9][6]='\\';
		
			lvl[10][6]='#';
			lvl[10][7]='\\';
			
			lvl[11][7]='#';
			lvl[11][8]='\\';
			
			lvl[12][8]='#';
			lvl[12][9]='\\';
			
			lvl[13][8]='#';
			lvl[13][10]='\\';
			
		}
	break;
	}
	
	case 3:
	{
		for(int i=0;i<width;i++)
		{
			if(i>=4&&i<=6)
			{
			  lvl[3][i]='#';
			  lvl[3][7]='\\';
			}
			
			lvl[4][7]='#';
			lvl[4][8]='\\';
			
			lvl[5][8]='#';
			lvl[5][9]='\\';
			
			lvl[6][9]='#';
			lvl[6][10]='\\';
		
			lvl[7][10]='#';
			lvl[7][11]='\\';
			
			lvl[8][11]='#';
			lvl[8][12]='\\';
			
			lvl[9][12]='#';
			lvl[9][13]='\\';
			
			lvl[10][13]='#';
			lvl[10][14]='#';
			lvl[10][15]='#';
		}
		
		for(int i=0;i<width;i++)
		{
			if(i>12)
				lvl[3][i]='#';
			if(i>15)
				lvl[6][i]='#';
			if(i>=0&&i<=4)
				lvl[6][i]='#';
			if(i>4&&i<8)
				lvl[8][i]='#';
			if(i>=0&&i<=5)
				lvl[11][i]='#';
		}	
	break;
	}
	case 4:
	{
		for(int i=0;i<width;i++)
		{
			if(i>=14&&i<=16)
			{
			  lvl[3][i]='#';
			  //lvl[3][15]='/';
			}
			
			lvl[3][15]='#';
			lvl[3][14]='/';
			
			lvl[4][14]='#';
			lvl[4][13]='/';
			
			lvl[5][13]='#';
			lvl[5][12]='/';
		
			lvl[6][12]='#';
			lvl[6][11]='/';
			
			lvl[7][11]='#';
			lvl[7][10]='/';
		
			lvl[8][10]='#';
			lvl[8][9]='/';
		
			lvl[9][9]='#';
			lvl[9][8]='/';
		
			lvl[10][8]='#';
			lvl[10][7]='#';
			lvl[10][6]='#';
		}
		
		for(int i=0;i<width;i++)
		{
			if(i>=0&&i<=5)
				lvl[4][i]='#';
			if(i>=1&&i<=4)
				lvl[7][i]='#';
			if(i>=0&&i<=3)
				lvl[10][i]='#';
			if(i>=13)
				lvl[10][i]='#';
			if(i>=15)
				lvl[7][i]='#';
		
		}	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	break;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	}
	
	//for first three lines space
		
	for(int i=0;i<width;i++)
	{
	lvl[0][i]=' ';
	lvl[1][i]=' ';
	//lvl[2][i]=' ';
	}
	
	//blocks on the last line
	for(int i=0;i<width;i++)
	{
		lvl[13][i]='#';
	}
	
	
	
	
	
	
	
	
	
	
		
		

	Event ev;
	//main loop
	while (window.isOpen())
	{

		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed) 
			{
				window.close();
			}

			if (ev.type == Event::KeyPressed)
			{
			}

		}

		//presing escape to close
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}

		window.clear();

		display_level(window, lvl, bgTex, bgSprite,
              blockTexture, blockSprite,
              triLeftSprite,triRightSprite,
              height, width, cell_size);

		player_gravity(lvl,offset_y,velocityY,onGround,gravity,terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth);
		PlayerSprite.setPosition(player_x, player_y);
		window.draw(PlayerSprite);

		window.display();
	}

	//stopping music and deleting level array
	lvlMusic.stop();
	for (int i = 0; i < height; i++)
	{
		delete[] lvl[i];
	}
	delete[] lvl;

	return 0;
}

