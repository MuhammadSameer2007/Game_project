#include <iostream>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace std;

int screen_x = 1136;
int screen_y = 896;

void display_level(RenderWindow &window, char **lvl, Texture &bgTex, Sprite &bgSprite, Texture &blockTexture, Sprite &blockSprite, const int height, const int width, const int cell_size)
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
		}
	}
}

void player_gravity(char **lvl, float &offset_y, float &velocityY, bool &onGround, const float &gravity, float &terminal_Velocity, float &player_x, float &player_y, const int cell_size, int &Pheight, int &Pwidth)
{
	offset_y = player_y;

	offset_y += velocityY;

	char bottom_left_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x) / cell_size];
	char bottom_right_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x + Pwidth) / cell_size];
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
		if (velocityY >= terminal_Velocity)
			velocityY = terminal_Velocity;
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

	int current = 0;

	Texture menubgtexture;
	if (!menubgtexture.loadFromFile("menu_bg1.png"))
	{
		cerr << "Failed to load menu_bg1.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite menubgsprite;
	menubgsprite.setTexture(menubgtexture);
	menubgsprite.setScale(
		float(screen_x) / menubgtexture.getSize().x,
		float(screen_y) / menubgtexture.getSize().y);

	Texture instbgtexture;
	if (!instbgtexture.loadFromFile("inst.png"))
	{
		cerr << "Failed to load inst.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite instbgsprite;
	instbgsprite.setTexture(instbgtexture);
	instbgsprite.setScale(
		float(screen_x) / instbgtexture.getSize().x,
		float(screen_y) / instbgtexture.getSize().y);

	Font font;
	if (!font.loadFromFile("Aileron.ttf"))
	{
		cout << "Failed to load arial.ttf\n";
		return -1;
	}

	Texture selectbgtexture;
	if (!selectbgtexture.loadFromFile("playerselection.png"))
	{
		cerr << "Failed to load playerselection.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite selectbgsprite;
	selectbgsprite.setTexture(selectbgtexture);
	selectbgsprite.setScale(
		float(screen_x) / selectbgtexture.getSize().x,
		float(screen_y) / selectbgtexture.getSize().y);

	//for player selection yellow

	Texture yellowbgtexture;
	if (!yellowbgtexture.loadFromFile("yellow.png"))
	{
		cerr << "Failed to load yellow.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite yellowbgsprite;
	yellowbgsprite.setTexture(yellowbgtexture);
	yellowbgsprite.setScale(3,3);
	yellowbgsprite.setPosition(220,400);

	//for player selection green
	Texture greenbgtexture;
	if (!greenbgtexture.loadFromFile("green.png"))
	{
		cerr << "Failed to load green.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite greenbgsprite;
	greenbgsprite.setTexture(greenbgtexture);
	greenbgsprite.setScale(3,3);
	greenbgsprite.setPosition(600,410);






	window.clear();
	window.draw(menubgsprite); // background first
							   // Menu options text
	const int numButtons = 3;
	
	string labels[numButtons] = {"START GAME", "INSTRUCTIONS", "QUIT"};
	
	Text buttonText[numButtons];

	

	for (int i = 0; i < numButtons; i++)
	{
		
		 buttonText[i].setFillColor(Color (255, 140, 0)); // darkblue
		
	

		// Button label
		buttonText[i].setFont(font);
		buttonText[i].setString(labels[i]);
		buttonText[i].setCharacterSize(40);
		buttonText[i].setFillColor(Color(255, 140, 0));
		buttonText[i].setPosition(550, 330 + i * 120);

		
	}
	buttonText[0].setPosition(450, 330 + 0 * 120);
	buttonText[1].setPosition(430, 330 + 1 * 120);
	buttonText[2].setPosition(520, 330 + 2 * 120);

	int selectedButton = 0;
	buttonText[selectedButton].setFillColor(Color(100, 100, 255)); // glow for selected

	int selectedplayer=0,numplayers=2;
	if(selectedplayer==0)
	{
		yellowbgsprite.setScale(4,4);
	}

	// level specifics
	const int cell_size = 64;
	const int height = 14;
	const int width = 18;
	char **lvl;

	// level and background textures and sprites
	Texture bgTex;
	Sprite bgSprite;
	Texture blockTexture;
	Sprite blockSprite;

	bgTex.loadFromFile("bg.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0, 0);

	blockTexture.loadFromFile("block1.png");
	blockSprite.setTexture(blockTexture);

	// Music initialisation
	Music lvlMusic;
	
	lvlMusic.openFromFile("mus.ogg");
	lvlMusic.setVolume(20);
	lvlMusic.play();
	lvlMusic.setLoop(true);

	// player data
	float player_x = 500;
	float player_y = 150;

	float speed = 5;

	const float jumpStrength = -20; // Initial jump velocity
	const float gravity = 1;		// Gravity acceleration

	bool isJumping = false; // Track if jumping

	bool up_collide = false;
	bool left_collide = false;
	bool right_collide = false;

	Texture yellowPlayerTexture;
	Sprite yellowPlayerSprite;

	Texture greenPlayerTexture;
	Sprite greenPlayerSprite;

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

	//for green player
	greenPlayerTexture.loadFromFile("greenplayer.png");
	greenPlayerSprite.setTexture(greenPlayerTexture);
	greenPlayerSprite.setScale(3, 3);
	greenPlayerSprite.setPosition(player_x, player_y);

	//for yellow player
	yellowPlayerTexture.loadFromFile("yellowplayer.png");
	yellowPlayerSprite.setTexture(yellowPlayerTexture);
	yellowPlayerSprite.setScale(3, 3);
	yellowPlayerSprite.setPosition(player_x, player_y);

	// creating level array
	lvl = new char *[height];
	for (int i = 0; i < height; i += 1)
	{
		lvl[i] = new char[width];
	}

	lvl[7][7] = '#';
	lvl[7][8] = '#';
	lvl[7][9] = '#';

	Event ev;
	// main loop
	while (window.isOpen())
	{
		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed)
				window.close();

			if (ev.type == Event::KeyPressed)
			{
				if (current == 0)
				{
					if (ev.key.code == Keyboard::Down)
					{
						buttonText[selectedButton].setFillColor(Color(255, 140, 0)); // reset color
						selectedButton = (selectedButton + 1) % numButtons;
						buttonText[selectedButton].setFillColor(Color(0, 0, 255)); // glow
					}
					else if (ev.key.code == Keyboard::Up)
					{
						 buttonText[selectedButton].setFillColor(Color(255, 140, 0));
						selectedButton = (selectedButton + numButtons - 1) % numButtons;
						buttonText[selectedButton].setFillColor(Color(0, 0, 255));
					}
					else if (ev.key.code == Keyboard::Enter)
					{
						if (selectedButton == 0)
							current = 2;
						else if (selectedButton == 1)
							current = 1;
						else if (selectedButton == 2)
							window.close();
					}
				}
				else if (current == 1)
				{
					if (ev.key.code == Keyboard::Escape)
					{
						current = 0;
					}
				}
				else if(current==2)
				{
					if(ev.key.code == Keyboard::Right)
					{
						greenbgsprite.setScale(4,4);
						selectedplayer=(selectedplayer + numplayers - 1) % numplayers;
						yellowbgsprite.setScale(3,3);
					}
					else if(ev.key.code == Keyboard::Left)
					{
						yellowbgsprite.setScale(4,4);
						selectedplayer=(selectedplayer + numplayers - 1) % numplayers;	
						greenbgsprite.setScale(3,3);
					}
					
				}
			}
		}

		// presing escape to close

		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.draw(menubgsprite);
			
		}

		window.clear();

		if (current == 0)
		{
			window.draw(menubgsprite);
			// window.draw(buttonbgsprite);
			for (int i = 0; i < numButtons; i++)
			{
				// window.draw(buttons[i]);
				window.draw(buttonText[i]);
			}
		}

		else if (current == 1)
		{
			window.draw(instbgsprite);
			if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape)
			{
				window.draw(menubgsprite);
				
				current = 0; // ← Go back to main menu
			}
		}

		else if(current==2)
		{
			window.draw(selectbgsprite);
			window.draw(yellowbgsprite);
			window.draw(greenbgsprite);

			if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape)
			{
				window.draw(menubgsprite);
				
				current = 0; // ← Go back to main menu
			}
			
			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				current=3;
			}
		}

		else if (current == 3)
		{
			if (current == 3 && ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape)
			{
				window.close();
			}
			
			display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);
			player_gravity(lvl, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth);
			greenPlayerSprite.setPosition(player_x, player_y);
			yellowPlayerSprite.setPosition(player_x, player_y);
			if(selectedplayer==0)
			{
				window.draw(yellowPlayerSprite);
				
			}
			else
			{
				window.draw(greenPlayerSprite);
			}

			
		}

		window.display();
	}

	// stopping music and deleting level array
	lvlMusic.stop();
	for (int i = 0; i < height; i++)
	{
		delete[] lvl[i];
	}
	delete[] lvl;

	return 0;
}
