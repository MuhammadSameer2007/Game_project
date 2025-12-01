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

void display_level(RenderWindow& window, char**lvl, Texture& bgTex,Sprite& bgSprite,Texture& blockTexture,Sprite& blockSprite, const int height, const int width, const int cell_size)
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

// Check GAME STATE

enum gamestate
{
Main_menu,
Instructions,
Game
};


int main()
{
        RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Resize);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	
	
		
		
	
	gamestate current=Main_menu;
	
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
        	float(screen_y) / menubgtexture.getSize().y
        );
        
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
        	float(screen_y) / instbgtexture.getSize().y
        );
	

	
	
	 Font font;
    if (!font.loadFromFile("Aileron.ttf"))
    {
        cout << "Failed to load arial.ttf\n";
        return -1;
    }
window.clear();
window.draw(menubgsprite); // background first
    // Menu options text
     const int numButtons = 3;
    string labels[numButtons] = { "START GAME", "INSTRUCTIONS", "QUIT" };
    RectangleShape buttons[numButtons];
    Text buttonText[numButtons];

    for (int i = 0; i < numButtons; i++)
    {
        // Rectangle button
        buttons[i].setSize(Vector2f(320, 80));
        buttons[i].setFillColor(Color(0, 0, 139)); //darkblue
        buttons[i].setOutlineThickness(10);
        buttons[i].setOutlineColor(Color(255, 20, 147));
        buttons[i].setPosition(430, 300 + i * 120);

        // Button label
        buttonText[i].setFont(font);
        buttonText[i].setString(labels[i]);
        buttonText[i].setCharacterSize(40);
        buttonText[i].setFillColor(Color(255, 140, 0));
        
        
        // Center text in rectangle
        FloatRect textRect = buttonText[i].getLocalBounds();
        buttonText[i].setOrigin(textRect.left + textRect.width / 2.0f,
                                textRect.top + textRect.height / 2.0f);
        buttonText[i].setPosition(
            buttons[i].getPosition().x + buttons[i].getSize().x / 2,
            buttons[i].getPosition().y + buttons[i].getSize().y / 2
        );
    }
	
	int selectedButton = 0;
    buttons[selectedButton].setFillColor(Color(100, 100, 255)); // glow for selected
  
	

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

	bgTex.loadFromFile("bg.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0,0);

	blockTexture.loadFromFile("block1.png");
	blockSprite.setTexture(blockTexture);

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

	lvl[7][7] = '#';
	lvl[7][8] = '#';
	lvl[7][9] = '#';
	
	

	Event ev;
	//main loop
	while (window.isOpen())
    {
        while (window.pollEvent(ev))
        {
            if (ev.type == Event::Closed) window.close();

            if (ev.type == Event::KeyPressed)
            {
                if (current== Main_menu)
                {
                    if (ev.key.code == Keyboard::Down)
                    {
                        buttons[selectedButton].setFillColor(Color(0, 0, 139)); // reset color
                        selectedButton = (selectedButton + 1) % numButtons;
                        buttons[selectedButton].setFillColor(Color(0, 0, 255)); // glow
                    }
                    else if (ev.key.code == Keyboard::Up)
                    {
                         buttons[selectedButton].setFillColor(Color(0, 0, 139));
                        selectedButton = (selectedButton + numButtons - 1) % numButtons;
                        buttons[selectedButton].setFillColor(Color(0, 0, 255));
                    }
                    else if (ev.key.code == Keyboard::Enter)
                    {
                         if (selectedButton == 0)
                            current = Game;
                        else if (selectedButton == 1)
                            current = Instructions;
                        else if (selectedButton == 2)
                            window.close();
                    }
                }
                else if (current == Instructions)
                {
                   if (ev.key.code == Keyboard::Escape)
            {
                current = Main_menu;   
            }
                        
                        
                }
            }
        }
		
		

		//presing escape to close
		
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.draw(menubgsprite);
		}

		window.clear();
		
		 if (current == Main_menu)
        {
          window.draw(menubgsprite);
            for (int i = 0; i < numButtons; i++)
            {
                window.draw(buttons[i]);
                window.draw(buttonText[i]);
            }
        }
        else if (current == Instructions)
{window.draw(instbgsprite);
    if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Escape)
    {
    	window.draw(menubgsprite);
        current = Main_menu;   // ← Go back to main menu
    }
}
	 else 
	 {
	 	if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
		display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);
		player_gravity(lvl,offset_y,velocityY,onGround,gravity,terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth);
		PlayerSprite.setPosition(player_x, player_y);
		window.draw(PlayerSprite);
	}

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

