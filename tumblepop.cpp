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

void applyVerticalPhysics(
	char **lvl,
	float &objX, float &objY, // position
	int &velY,				  // vertical velocity
	bool &onGround,			  // output: is object standing?
	float gravity, float terminalVel,
	float objWidth, float objHeight,
	int cell_size,
	int screen_x, int screen_y)
{
	// Predict next Y
	float nextY = objY + velY;

	// Compute tile row below sprite bottom
	int bottomRow = (int)(nextY + objHeight) / cell_size;

	// Clamp to valid rows
	if (bottomRow >= 14)
		bottomRow = 13;
	if (bottomRow < 0)
		bottomRow = 0;

	// Columns for bottom-left, bottom-mid, bottom-right
	int leftCol = (int)(objX) / cell_size;
	int midCol = (int)(objX + objWidth / 2) / cell_size;
	int rightCol = (int)(objX + objWidth) / cell_size;

	// Safe clamp
	if (leftCol < 0)
		leftCol = 0;
	if (midCol < 0)
		midCol = 0;
	if (rightCol < 0)
		rightCol = 0;

	if (rightCol >= 18)
		rightCol = 17;
	if (midCol >= 18)
		midCol = 17;
	if (leftCol >= 18)
		leftCol = 17;

	char bL = lvl[bottomRow][leftCol];
	char bM = lvl[bottomRow][midCol];
	char bR = lvl[bottomRow][rightCol];

	// --- FLOOR COLLISION ---
	if (velY >= 0 && (bL == '#' || bM == '#' || bR == '#'))
	{
		onGround = true;
		velY = 0;
	}
	else
	{
		// Apply motion
		objY = nextY;
		onGround = false;

		// Apply gravity
		velY += gravity;
		if (velY > terminalVel)
			velY = terminalVel;
	}

	// --- Screen boundaries ---
	if (objY < 0)
	{
		objY = 0;
		velY = 0;
	}

	if (objY + objHeight > screen_y)
	{
		objY = screen_y - objHeight;
		onGround = true;
		velY = 0;
	}
}

void initGhosts(Texture &ghostTexture, Sprite ghostSpriteArr[], float ghostX[],
				float ghostY[], bool ghostRight[], int ghostCount, float ghostWidth, float ghostHeight,
				bool ghostOnGround[])
{
	for (int i = 0; i < ghostCount; i++)
	{
		ghostSpriteArr[i].setTexture(ghostTexture);

		// Initial positions based on your game design
		switch (i)
		{
		case 0:
			ghostX[i] = 129;
			ghostY[i] = 195.5;
			ghostRight[i] = true;
			break;
		case 1:
			ghostX[i] = 915.2;
			ghostY[i] = 195.5;
			ghostRight[i] = false;
			break;
		case 2:
			ghostX[i] = 1;
			ghostY[i] = 387.5;
			ghostRight[i] = true;
			break;
		case 3:
			ghostX[i] = 530.2;
			ghostY[i] = 387.5;
			ghostRight[i] = false;
			break;
		case 4:
			ghostX[i] = 833;
			ghostY[i] = 387.5;
			ghostRight[i] = true;
			break;
		case 5:
			ghostX[i] = 211.2;
			ghostY[i] = 579.5;
			ghostRight[i] = false;
			break;
		case 6:
			ghostX[i] = 513;
			ghostY[i] = 579.5;
			ghostRight[i] = true;
			break;
		case 7:
			ghostX[i] = 1043.2;
			ghostY[i] = 579.5;
			ghostRight[i] = false;
			break;
		default:
			break;
		}

		// Direction Initial
		if (ghostRight[i])
		{
			ghostSpriteArr[i].setScale(-1.5f, 1.1f);
			ghostX[i] += ghostWidth; // fix to avoid shifting left
		}
		else
		{
			ghostSpriteArr[i].setScale(1.5f, 1.1f);
		}

		// Applying Initial Position
		ghostSpriteArr[i].setPosition(ghostX[i], ghostY[i]);
	}
}

void updateGhosts(char **lvl, Sprite ghostSpriteArr[], Texture ghostTexture, float ghostX[], float ghostY[], bool ghostRight[], bool ghostIsThrown[], int ghostRandom[], int ghostCount, float ghostSpeed, float ghostWidth, float ghostHeight, int cell_size, int screen_x, bool ghostDead[], bool ghostCaptured[], bool ghostInVac[])
{

	for (int i = 0; i < ghostCount; i++)
	{
		bool old = ghostRight[i];

		if (ghostDead[i] || ghostCaptured[i] || ghostInVac[i] || ghostIsThrown[i])
		{
			ghostSpriteArr[i].setPosition(ghostX[i], ghostY[i]);
			continue;
		}

		// Checking PLatform
		if (ghostRight[i])
		{
			// Check next tile (right side bottom)
			if (lvl[(int)(ghostY[i] + ghostHeight) / cell_size + 1][(int)(ghostX[i] + ghostWidth) / cell_size] != '#')
				ghostRight[i] = false;
		}
		else
		{
			// Check next tile (left side bottom)
			if (lvl[(int)(ghostY[i] + ghostHeight) / cell_size + 1][(int)(ghostX[i] - 1) / cell_size] != '#')
				ghostRight[i] = true;
		}

		// Next Position of Ghost
		if (ghostRight[i])
			ghostX[i] += ghostSpeed;
		else
			ghostX[i] -= ghostSpeed;

		// Horizontal Screen Collision
		if (ghostX[i] < 0 && !ghostRight[i])
		{
			ghostRight[i] = true;
			ghostX[i] += ghostSpeed;
		}

		if (ghostX[i] > screen_x - ghostWidth && ghostRight[i])
		{
			ghostRight[i] = false;
			ghostX[i] -= ghostSpeed;
		}

		// Random change of direction
		if (old == ghostRight[i] && rand() % 200 == 0 && ghostRandom[i] == 0)
		{
			ghostRight[i] = !ghostRight[i];
			ghostRandom[i] = 240; // cooldown
		}

		// Flipping of sprite (if direction changes)
		if (old != ghostRight[i])
		{
			if (ghostRight[i])
			{
				ghostSpriteArr[i].setScale(-1.5f, 1.1f);
				ghostX[i] += ghostWidth;
			}
			else
			{
				ghostSpriteArr[i].setScale(1.5f, 1.1f);
				ghostX[i] -= ghostWidth;
			}
		}

		// Random Cooldown
		if (ghostRandom[i] > 0)
			ghostRandom[i]--;

		// Apply New Position
		ghostSpriteArr[i].setPosition(ghostX[i], ghostY[i]);
	}
}

void initSkeletons(Texture &skTexture, Sprite skSprite[],
				   float skX[], float skY[], bool skRight[],
				   int skVelocityY[], bool skOnGround[],
				   int skCooldown[], int skCount, float skWidth, float skHeight)
{
	for (int i = 0; i < skCount; i++)
	{
		skSprite[i].setTexture(skTexture);
		skSprite[i].setScale(1.0f, 1.1f);

		// Starting positions (you can change)
		switch (i)
		{
		case 0:
			skX[i] = 1;
			skY[i] = (7 * 64) - skHeight;
			skRight[i] = true;
			break;
		case 1:
			skX[i] = 1081;
			skY[i] = (10 * 64) - skHeight;
			skRight[i] = false;
			break;
		case 2:
			skX[i] = 1;
			skY[i] = (13 * 64) - skHeight;
			skRight[i] = true;
			break;
		case 3:
			skX[i] = 1081;
			skY[i] = (13 * 64) - skHeight;
			skRight[i] = false;
			break;
		}

		// Flip sprite if facing right
		if (skRight[i])
		{
			skSprite[i].setScale(-1.0f, 1.1f);
			skX[i] += skWidth;
		}

		skY[i] -= 20; // lift slightly so they start above the floor
		skVelocityY[i] = 0;
		skOnGround[i] = false;
		skCooldown[i] = 0;

		skSprite[i].setPosition(skX[i], skY[i]);
	}
}

bool canSkeletonMove(float newX, float skY, int skWidth, int skHeight,
					 char **lvl, int cell_size)
{
	int topRow = (int)(skY) / cell_size;
	int midRow = (int)(skY + skHeight / 2) / cell_size;
	int bottomRow = (int)(skY + skHeight) / cell_size;

	int leftCol = (int)(newX) / cell_size;
	int rightCol = (int)(newX + skWidth) / cell_size;

	char tl = lvl[topRow][leftCol];
	char ml = lvl[midRow][leftCol];
	char bl = lvl[bottomRow][leftCol];

	char tr = lvl[topRow][rightCol];
	char mr = lvl[midRow][rightCol];
	char br = lvl[bottomRow][rightCol];

	if (tl == '#' || ml == '#' || bl == '#')
		return false;
	if (tr == '#' || mr == '#' || br == '#')
		return false;

	return true;
}

void updateSkeletons(char **lvl,
					 Sprite skSprite[], Texture skeletonTexture, float skX[], float skY[],
					 bool skRight[], bool skThrown[], int skVelocityY[],
					 bool skOnGround[], int skCooldown[],
					 int skCount, float skSpeed, float jumpStrength,
					 float gravity, float terminal_Velocity,
					 int cell_size, int screen_x, float skWidth, float skHeight, bool skeletonDead[], bool skeletonCaptured[], bool skeletonInVac[])
{
	for (int i = 0; i < skCount; i++)
	{

		bool oldDir = skRight[i];

		if (skeletonDead[i] || skeletonCaptured[i] || skeletonInVac[i] || skThrown[i])
		{
			skSprite[i].setPosition(skX[i], skY[i]);
			continue;
		}

		// Detect top & bottom platform
		int currentRow = (int)(skY[i] + skHeight) / cell_size;

		bool onTopPlatform = (currentRow == 4);
		bool onBottomPlatform = (currentRow == 13);

		// Random Horizontal Direction Change
		if (skCooldown[i] == 0 && rand() % 480 == 0)
		{
			skRight[i] = !skRight[i];
			skCooldown[i] = 480;
		}

		if (skCooldown[i] > 0)
			skCooldown[i]--;

		// Horizontal Movement
		float movX = skRight[i] ? skSpeed : -skSpeed;
		float newX = skX[i] + movX;

		// TRY MOVE — ONLY move if no block collision
		if (canSkeletonMove(newX, skY[i], skWidth, skHeight, lvl, cell_size))
		{
			skX[i] = newX;
		}
		else
		{
			// DO NOT change direction
			// DO NOT move
			// skeleton stays stuck until cooldown flips direction
		}

		// Screen Collission
		if (skX[i] <= 0)
		{
			skX[i] = 0;
			skRight[i] = true;
		}
		if (skX[i] + skWidth >= screen_x)
		{
			skX[i] = screen_x - skWidth;
			skRight[i] = false;
		}

		// Random Jump
		if (skOnGround[i] && !onTopPlatform && rand() % 200 == 0)
		{
			skVelocityY[i] = jumpStrength;
			skOnGround[i] = false;
		}

		applyVerticalPhysics(
			lvl,
			skX[i], skY[i],
			skVelocityY[i],
			skOnGround[i],
			gravity, terminal_Velocity,
			skWidth, skHeight,
			cell_size,
			screen_x, screen_y);

		// Flipping of Sprite
		if (oldDir != skRight[i])
		{
			if (skRight[i])
			{
				skSprite[i].setScale(-1.1f, 1.1f);
				skX[i] += skWidth;
			}
			else
			{
				skSprite[i].setScale(1.1f, 1.1f);
			}
		}

		// Setting New Positions
		skSprite[i].setPosition(skX[i], skY[i]);
	}
}

bool enemyIsInFront(bool facing_right, bool enemy_right, float player_x, float enemy_x, float enemyWidth)
{
	bool result;
	if (facing_right)
	{
		if (enemy_right)
			result = enemy_x - enemyWidth >= player_x && enemy_x - enemyWidth <= player_x + 256;
		else if (!enemy_right)
			result = enemy_x > player_x && enemy_x <= player_x + 256;
	}
	else if (!facing_right)
	{
		if (enemy_right)
			result = enemy_x <= player_x && enemy_x >= player_x - 256;
		else if (!enemy_right)
			result = enemy_x + enemyWidth <= player_x && enemy_x + enemyWidth >= player_x - 256;
	}
	return result;
}

void updateGhostSuction(float player_x, float player_y, bool facing_right, char **lvl,
						float ghostX[], float ghostY[],
						bool ghostCaptured[], bool ghostDead[], bool ghostThrown[],
						bool ghostInVac[],
						bool ghostRight[],
						int &playerCapturedCount,
						const int ghostCount,
						float ghostWidth, float ghostHeight,
						int cell_size, int &capTop, int capEnemyID[], int capEnemyType[],int &playerscore)
{

	for (int i = 0; i < ghostCount; i++)
	{
		if (ghostDead[i] || ghostCaptured[i] || ghostThrown[i])
			continue;

		// ------- 1. Same vertical platform -------
		int playerRow = (int)(player_y + 40) / cell_size;
		int ghostRow = (int)(ghostY[i] + ghostHeight) / cell_size;

		if (playerRow != ghostRow)
			continue;

		// ------- 2. Horizontal distance within 256 px -------
		float dist = fabs((ghostX[i] + ghostWidth / 2) - (player_x + 48));
		if (dist > 256)
			continue;

		if (!enemyIsInFront(facing_right, ghostRight[i], player_x, ghostX[i], ghostWidth))
			continue; // ← enemy is behind player, skip

		// ------- 3. SUCTION: move ghost towards player -------
		float speed = 3.0f;
		ghostInVac[i] = true;

		if (ghostX[i] < player_x)
			ghostX[i] += speed;
		else
			ghostX[i] -= speed;

		// ghost suction logic
		if (facing_right)
		{
			if (ghostRight[i] && player_x >= ghostX[i] - ghostWidth)
			{
				capTop++;
				capEnemyID[capTop] = i;	  // index
				capEnemyType[capTop] = 1; // or false for skeleton

				ghostCaptured[i] = true;
				ghostInVac[i] = false;
				ghostX[i] = -9999;
				ghostY[i] = -9999;
				playerCapturedCount++;
			}
			else if (!ghostRight[i] && player_x >= ghostX[i])
			{
				capTop++;
				capEnemyID[capTop] = i;	  // index
				capEnemyType[capTop] = 1; // or false for skeleton

				ghostCaptured[i] = true;
				ghostInVac[i] = false;
				ghostX[i] = -9999;
				ghostY[i] = -9999;
				playerCapturedCount++;
			}
			

		}
		else if (!facing_right)
		{
			if (ghostRight[i] && player_x <= ghostX[i])
			{
				capTop++;
				capEnemyID[capTop] = i;	  // index
				capEnemyType[capTop] = 1; // or false for skeleton

				ghostCaptured[i] = true;
				ghostInVac[i] = false;
				ghostX[i] = -9999;
				ghostY[i] = -9999;
				playerCapturedCount++;
			}
			else if (!ghostRight[i] && player_x <= ghostX[i] + ghostWidth)
			{
				capTop++;
				capEnemyID[capTop] = i;	  // index
				capEnemyType[capTop] = 1; // or false for skeleton

				ghostCaptured[i] = true;
				ghostInVac[i] = false;
				ghostX[i] = -9999;
				ghostY[i] = -9999;
				playerCapturedCount++;
			}
		}

		//**************************************CHECKED IF SKELETON CAPTURED***************************************** */


		if(ghostCaptured[i]==true)
		{
			playerscore+=50;
		}
	}
}

void updateSkeletonSuction(float player_x, float player_y,
						   char **lvl,
						   float skX[], float skY[],
						   bool skeletonCaptured[], bool skeletonDead[], bool skeletonThrown[],
						   int &playerCapturedCount,
						   int skeletonCount,
						   float skWidth, float skHeight,
						   int cell_size, bool skeletonInVac[], bool skeletonRight[], bool facing_right, int &capTop, int capEnemyID[], int capEnemyType[],int &playerscore)
{

	for (int i = 0; i < skeletonCount; i++)
	{
		if (skeletonDead[i])
			continue;
		if (skeletonCaptured[i] || skeletonThrown[i])
			continue;

		// ------ 1. Same vertical platform ------
		int playerRow = (int)(player_y + 40) / cell_size;
		int skRow = (int)(skY[i] + skHeight) / cell_size;

		if (playerRow != skRow)
			continue;

		// ------ 2. Horizontal distance ≤ 256 ------
		float dist = fabs((skX[i] + skWidth / 2) - (player_x + 48));
		if (dist > 256)
			continue;

		if (!enemyIsInFront(facing_right, skeletonRight[i], player_x, skX[i], skWidth))
			continue;

		// ------ 3. Suction pull ------
		float pullSpeed = 3.0f;
		skeletonInVac[i] = true;
		if (skX[i] < player_x)
			skX[i] += pullSpeed;
		else
			skX[i] -= pullSpeed;

		// ------ 4. Captured ------
		if (facing_right)
		{
			if (skeletonRight[i] && player_x >= skeletonRight[i] - skWidth)
			{
				capTop++;
				capEnemyID[capTop] = i; // index
				capEnemyType[capTop] = 2;

				skeletonCaptured[i] = true;
				skeletonInVac[i] = false;
				skX[i] = -9999;
				skY[i] = -9999;
				playerCapturedCount++;
			}
			else if (!skeletonRight[i] && player_x >= skX[i])
			{
				capTop++;
				capEnemyID[capTop] = i; // index
				capEnemyType[capTop] = 2;

				skeletonCaptured[i] = true;
				skeletonInVac[i] = false;
				skX[i] = -9999;
				skY[i] = -9999;
				playerCapturedCount++;
			}
		}
		else if (!facing_right)
		{
			if (skeletonRight[i] && player_x <= skX[i])
			{
				capTop++;
				capEnemyID[capTop] = i; // index
				capEnemyType[capTop] = 2;

				skeletonCaptured[i] = true;
				skeletonInVac[i] = false;
				skX[i] = -9999;
				skY[i] = -9999;
				playerCapturedCount++;
			}
			else if (!skeletonRight[i] && player_x <= skX[i] + skWidth)
			{
				capTop++;
				capEnemyID[capTop] = i; // index
				capEnemyType[capTop] = 2;

				skeletonCaptured[i] = true;
				skeletonInVac[i] = false;
				skX[i] = -9999;
				skY[i] = -9999;
				playerCapturedCount++;
			}
		}

		//**************************************CHECKED IF SKELETON CAPTURED***************************************** */
		if(skeletonCaptured[i]==true)
		{
			playerscore+=75;
		}
	}
}

void throwLastCapturedEnemy(
	float player_x, float player_y, float playerWidth, float playerHeight, bool facing_right,
	float ghostX[], float ghostY[],
	bool ghostThrown[], int ghostThrowDir[],
	bool ghostCaptured[], bool ghostOnGround[], int ghostVelocityY[],
	int ghostThrowAnimIndex[], float ghostThrowAnimTimer[], int ghostThrownVerDir[], float ghostHeight,
	float ghostWidth,

	float skX[], float skY[],
	bool skThrown[], int skThrowDir[],
	bool skCaptured[], bool skOnGround[], int skVelocityY[], int skThrownVerDir[], float skWidth, float skHeight,

	int capEnemyID[], int capEnemyType[], int &capTop, int &playerCapturedCount)
{
	if (capTop < 0)
	{
		capTop = -1;
		return;
	}

	int id = capEnemyID[capTop];
	int enemyType = capEnemyType[capTop];
	capTop--;
	playerCapturedCount--;

	switch (enemyType)
	{
	case 1:
		if (ghostThrowDir[id] == 0)
		{
			if (facing_right)
			{
				ghostX[id] = player_x + 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostThrowDir[id] = 2;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
			else if (!facing_right)
			{
				ghostX[id] = player_x - 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostThrowDir[id] = 4;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
		}
		else if (ghostThrowDir[id] == 1)
		{
			if (facing_right)
			{
				ghostX[id] = player_x + 10;
				ghostY[id] = player_y + 10;
				ghostOnGround[id] = false;
				ghostVelocityY[id] = -20;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
				ghostThrownVerDir[id] = 1;
			}
			else if (!facing_right)
			{
				ghostX[id] = player_x - 10;
				ghostY[id] = player_y + 20;
				ghostOnGround[id] = false;
				ghostVelocityY[id] = -20;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
				ghostThrownVerDir[id] = 2;
			}
		}
		else if (ghostThrowDir[id] == 2)
		{
			if (facing_right)
			{
				ghostX[id] = player_x + 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
			else if (!facing_right)
			{
				ghostX[id] = player_x + playerWidth + 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
		}
		else if (ghostThrowDir[id] == 3)
		{
			if (facing_right)
			{
				ghostX[id] = player_x + 10;
				ghostY[id] = player_y + playerHeight + 64;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostOnGround[id] = false;
				ghostVelocityY[id] = 0;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
				ghostThrownVerDir[id] = 1;
			}
			else if (!facing_right)
			{
				ghostX[id] = player_x - 10;
				ghostY[id] = player_y + 64;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostOnGround[id] = false;
				ghostVelocityY[id] = 0;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
				ghostThrownVerDir[id] = 2;
			}
		}
		else if (ghostThrowDir[id] == 4)
		{
			if (facing_right)
			{
				ghostX[id] = player_x - playerWidth - 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
			else if (!facing_right)
			{
				ghostX[id] = player_x - 10;
				ghostY[id] = player_y + playerHeight - ghostHeight;
				ghostCaptured[id] = false;
				ghostThrown[id] = true;
				ghostThrowAnimIndex[id] = 0;
				ghostThrowAnimTimer[id] = 0;
			}
		}
		break;

	case 2:
		if (skThrowDir[id] == 0)
		{
			if (facing_right)
			{
				skX[id] = player_x + 10;
				skY[id] = player_y + playerHeight - skHeight;
				skThrowDir[id] = 2;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
			else
			{
				skX[id] = player_x - 10;
				skY[id] = player_y + playerHeight - skHeight;
				skThrowDir[id] = 4;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
		}
		else if (skThrowDir[id] == 1)
		{
			if (facing_right)
			{
				skX[id] = player_x + 10;
				skY[id] = player_y + 10;
				skOnGround[id] = false;
				skVelocityY[id] = -20;
				skCaptured[id] = false;
				skThrown[id] = true;
				skThrownVerDir[id] = 1;
			}
			else
			{
				skX[id] = player_x - 10;
				skY[id] = player_y + 20;
				skOnGround[id] = false;
				skVelocityY[id] = -20;
				skCaptured[id] = false;
				skThrown[id] = true;
				skThrownVerDir[id] = 2;
			}
		}
		else if (skThrowDir[id] == 2)
		{
			if (facing_right)
			{
				skX[id] = player_x + 10;
				skY[id] = player_y + playerHeight - skHeight;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
			else
			{
				skX[id] = player_x + playerWidth + 10;
				skY[id] = player_y + playerHeight - skHeight;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
		}
		else if (skThrowDir[id] == 3)
		{
			if (facing_right)
			{
				skX[id] = player_x + 10;
				skY[id] = player_y + playerHeight + 64;
				skCaptured[id] = false;
				skThrown[id] = true;
				skOnGround[id] = false;
				skVelocityY[id] = 0;
				skThrownVerDir[id] = 1;
			}
			else
			{
				skX[id] = player_x - 10;
				skY[id] = player_y + 64;
				skCaptured[id] = false;
				skThrown[id] = true;
				skOnGround[id] = false;
				skVelocityY[id] = 0;
				skThrownVerDir[id] = 2;
			}
		}
		else if (skThrowDir[id] == 4)
		{
			if (facing_right)
			{
				skX[id] = player_x - playerWidth - 10;
				skY[id] = player_y + playerHeight - skHeight;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
			else
			{
				skX[id] = player_x - 10;
				skY[id] = player_y + playerHeight - skHeight;
				skCaptured[id] = false;
				skThrown[id] = true;
			}
		}

	default:
		break;
	}
}

void animateCharacter(
	Sprite &sprite,
	Texture frames[],  // array of textures
	int frameCount,	   // number of frames
	int &frameIndex,   // current frame index (counter)
	float &frameTimer, // timer for animation speed
	float frameSpeed,  // how fast to animate
	bool facingRight,  // flip sprite?
	float widthScale,
	float heightScale)
{
	frameTimer += frameSpeed;

	if (frameTimer >= 1.0f)
	{
		frameTimer = 0;
		frameIndex++;

		if (frameIndex >= frameCount)
			frameIndex = 0;
	}

	sprite.setTexture(frames[frameIndex]);

	// Handle flipping
	if (facingRight)
		sprite.setScale(-widthScale, heightScale);
	else
		sprite.setScale(widthScale, heightScale);
}



//***************************************ADDED COMBAT COUNTER IN EACH IF OF OVERLAP AND AT THE END CHECKED COMBATCOUTER************************************************* */


void updateThrownEnemies(
	char **lvl, int screen_x, int screen_y, int cell_size,
	float player_x, float player_y, float playerWidth, float playerHeight, bool facing_right,
	float ghostX[], float ghostY[],
	bool ghostThrown[], int ghostThrowDir[], int ghostThrownVerDir[],
	bool ghostCaptured[], bool ghostOnGround[], int ghostVelocityY[], int ghostCount, bool ghostDead[],
	bool ghostInVac[], bool ghostRight[], float ghostWidth, float ghostHeight,

	float skX[], float skY[], int skCount, float skWidth, float skHeight,
	bool skThrown[], int skThrowDir[], int skThrownVerDir[], bool skInVac[],
	bool skCaptured[], bool skOnGround[], int skVelocityY[], bool skDead[],int &playerscore)
{

	int combatcount=0;


	for (int i = 0; i < ghostCount; i++)
	{
		if (ghostThrown[i])
		{
			if (ghostX[i] >= screen_x - 82 || ghostX[i] <= 0)
			{
				ghostThrown[i] = false;
				ghostDead[i] = true;
				ghostX[i] = -9999;
				ghostY[i] = -9999;
				continue;
			}

			if (ghostThrowDir[i] == 1)
			{
				ghostX[i] += ghostThrownVerDir[i] == 1 ? +5 : -5;
				applyVerticalPhysics(
					lvl, ghostX[i], ghostY[i], (ghostVelocityY[i]),
					ghostOnGround[i], 1.0f, 20.0f, 96.0f, 102.0f, cell_size,
					screen_x, screen_y);
			}
			else if (ghostThrowDir[i] == 2)
			{
				ghostX[i] += 5;
				applyVerticalPhysics(
					lvl, ghostX[i], ghostY[i], (ghostVelocityY[i]),
					ghostOnGround[i], 1.0f, 20.0f, 96.0f, 102.0f, cell_size,
					screen_x, screen_y);
			}
			else if (ghostThrowDir[i] == 3)
			{
				ghostX[i] += ghostThrownVerDir[i] == 1 ? +5 : -5;
				applyVerticalPhysics(
					lvl, ghostX[i], ghostY[i], (ghostVelocityY[i]),
					ghostOnGround[i], 1.0f, 20.0f, 96.0f, 102.0f, cell_size,
					screen_x, screen_y);
			}
			else if (ghostThrowDir[i] == 4)
			{
				ghostX[i] -= 5;
				applyVerticalPhysics(
					lvl, ghostX[i], ghostY[i], (ghostVelocityY[i]),
					ghostOnGround[i], 1.0f, 20.0f, 96.0f, 102.0f, cell_size,
					screen_x, screen_y);
			}

			for (int j = 0; j < ghostCount; j++)
			{
				if (i == j)
					continue; // skip self
				if (ghostDead[j])
					continue;
				if (ghostCaptured[j] || ghostInVac[j])
					continue;

				// ONLY trigger if THIS ghost is thrown AND target is NOT thrown
				if (!ghostThrown[j])
				{
					bool overlap =
						ghostX[i] < ghostX[j] + ghostWidth &&
						ghostX[i] + ghostWidth > ghostX[j] &&
						ghostY[i] < ghostY[j] + ghostHeight &&
						ghostY[i] + ghostHeight > ghostY[j];

					if (overlap)
					{
						combatcount+=1;

						ghostThrown[j] = true;
						ghostCaptured[j] = false;
						ghostInVac[j] = false;
						ghostDead[j] = false;

						ghostThrowDir[j] = ghostThrowDir[i];
						ghostThrownVerDir[j] = ghostThrownVerDir[i];

						// give same velocity
						ghostVelocityY[j] = ghostVelocityY[i];

						// OPTIONAL slight push
						ghostX[j] += (ghostThrowDir[i] == 4 ? -5 : 5);
					}
				}
			}
			
			// ----- GHOST -> hits SKELETON -----
			for (int s = 0; s < skCount; s++)
			{
				if (skDead[s])
					continue;
				if (skCaptured[s] || skInVac[s])
					continue;
				if (skThrown[s])
					continue; // already projectile

				bool overlap =
					ghostX[i] < skX[s] + skWidth &&
					ghostX[i] + ghostWidth > skX[s] &&
					ghostY[i] < skY[s] + skHeight &&
					ghostY[i] + ghostHeight > skY[s];

				if (overlap)
				{
					combatcount+=1;
					skThrown[s] = true;
					skCaptured[s] = false;
					skInVac[s] = false;
					skDead[s] = false;

					skThrowDir[s] = ghostThrowDir[i];
					skThrownVerDir[s] = ghostThrownVerDir[i];
					skVelocityY[s] = ghostVelocityY[i];

					// nudging away
					skX[s] += (ghostThrowDir[i] == 4 ? -5 : 5);
				}

				
			}
		}
	}

	for (int i = 0; i < skCount; i++)
	{
		if (skThrown[i])
		{
			if (skX[i] >= screen_x - 82 || skX[i] <= 0)
			{
				skThrown[i] = false;
				skDead[i] = true;
				skX[i] = -9999;
				skY[i] = -9999;
				continue;
			}

			if (skThrowDir[i] == 1)
			{
				skX[i] += (skThrownVerDir[i] == 1 ? 5 : -5);
				applyVerticalPhysics(lvl, skX[i], skY[i], skVelocityY[i],
									 skOnGround[i], 1.0f, 20.0f, skWidth, skHeight, cell_size, screen_x, screen_y);
			}
			else if (skThrowDir[i] == 2)
			{
				skX[i] += 5;
				applyVerticalPhysics(lvl, skX[i], skY[i], skVelocityY[i],
									 skOnGround[i], 1.0f, 20.0f, skWidth, skHeight, cell_size, screen_x, screen_y);
			}
			else if (skThrowDir[i] == 3)
			{
				skX[i] += (skThrownVerDir[i] == 1 ? 5 : -5);
				applyVerticalPhysics(lvl, skX[i], skY[i], skVelocityY[i],
									 skOnGround[i], 1.0f, 20.0f, skWidth, skHeight, cell_size, screen_x, screen_y);
			}
			else if (skThrowDir[i] == 4)
			{
				skX[i] -= 5;
				applyVerticalPhysics(lvl, skX[i], skY[i], skVelocityY[i],
									 skOnGround[i], 1.0f, 20.0f, skWidth, skHeight, cell_size, screen_x, screen_y);
			}

			for (int g = 0; g < ghostCount; g++)
			{
				if (ghostDead[g])
					continue;
				if (ghostCaptured[g] || ghostInVac[g])
					continue;
				if (ghostThrown[g])
					continue;

				bool overlap =
					skX[i] < ghostX[g] + ghostWidth &&
					skX[i] + skWidth > ghostX[g] &&
					skY[i] < ghostY[g] + ghostHeight &&
					skY[i] + skHeight > ghostY[g];

				if (overlap)
				{
					combatcount+=1;
					ghostThrown[g] = true;
					ghostCaptured[g] = false;
					ghostInVac[g] = false;
					ghostDead[g] = false;

					ghostThrowDir[g] = skThrowDir[i];
					ghostThrownVerDir[g] = skThrownVerDir[i];
					ghostVelocityY[g] = skVelocityY[i];

					ghostX[g] += (skThrowDir[i] == 4 ? -5 : 5);
				}

				if(ghostCaptured[g]==true)
				{
					playerscore+=200;
				}
			}

			for (int s = 0; s < skCount; s++)
			{
				if (i == s)
					continue;
				if (skDead[s])
					continue;
				if (skCaptured[s] || skInVac[s])
					continue;
				if (skThrown[s])
					continue;

				bool overlap =
					skX[i] < skX[s] + skWidth &&
					skX[i] + skWidth > skX[s] &&
					skY[i] < skY[s] + skHeight &&
					skY[i] + skHeight > skY[s];

				if (overlap)
				{
					combatcount+=1;
					skThrown[s] = true;
					skCaptured[s] = false;
					skInVac[s] = false;
					skDead[s] = false;

					skThrowDir[s] = skThrowDir[i];
					skThrownVerDir[s] = skThrownVerDir[i];
					skVelocityY[s] = skVelocityY[i];

					skX[s] += (skThrowDir[i] == 4 ? -5 : 5);
				}
				
			}
		}
	}

	if(combatcount==0)
		playerscore+=0;
	else if(combatcount==1)
		playerscore+=200;
	else if(combatcount>1)
		playerscore+=500;
}

void updatePlayerLifeSystem(
	float &player_x, float &player_y, bool facing_right,
	float playerWidth, float playerHeight,
	float defaultX, float defaultY,
	int &playerLives,
	bool &playerDead,
	int &playerRespawnTimer,

	// Ghost Arrays
	float ghostX[], float ghostY[], bool ghostRight[],
	bool ghostDead[], bool ghostCaptured[], bool ghostInVac[], bool ghostThrown[],
	int ghostCount, float ghostWidth, float ghostHeight,

	// Skeleton Arrays
	float skX[], float skY[], bool skRight[],
	bool skDead[], bool skCaptured[], bool skInVac[], bool skThrown[],
	int skCount, float skWidth, float skHeight,int &playerscore)
{
	// -----------------------------
	// 1. PLAYER IS DEAD → WAIT
	// -----------------------------
	if (playerDead)
	{
		
		playerRespawnTimer--;

		if (playerRespawnTimer <= 0)
		{
			playerDead = false;
			player_x = defaultX;
			player_y = defaultY;
		}

		return;
	}

	// -----------------------------
	// 2. ACTIVE PLAYER CHECK COLLISION
	// -----------------------------
	float pL = (facing_right ? player_x - playerWidth : player_x);
	float pR = player_x + (facing_right ? 0 : playerWidth);
	float pT = player_y;
	float pB = player_y + playerHeight;

	// -----------------------------
	// GHOST collision
	// -----------------------------
	for (int i = 0; i < ghostCount; i++)
	{
		if (ghostDead[i] || ghostThrown[i])
			continue;
		if (ghostCaptured[i] || ghostInVac[i])
			continue; // <-- NEW: sucking = SAFE

		float gL = (ghostRight[i] ? ghostX[i] - ghostWidth : ghostX[i]);
		float gR = ghostX[i] + (ghostRight[i] ? 0 : ghostWidth);
		float gT = ghostY[i];
		float gB = ghostY[i] + ghostHeight;

		bool overlap =
			pL < gR && pR > gL &&
			pT < gB && pB > gT;

		if (overlap)
		{
			playerLives--;
			playerDead = true;
			playerRespawnTimer = 60;
			playerscore-=50;
			return;
		}
	}

	// -----------------------------
	// SKELETON collision
	// -----------------------------
	for (int i = 0; i < skCount; i++)
	{
		if (skDead[i] || skThrown[i])
			continue;
		if (skCaptured[i] || skInVac[i])
			continue; // <-- NEW: sucking = SAFE

		float sL = (skRight[i] ? skX[i] - skWidth : skX[i]);
		float sR = skX[i] + (skRight[i] ? 0 : skWidth);
		float sT = skY[i];
		float sB = skY[i] + skHeight;

		bool overlap =
			pL < sR && pR > sL &&
			pT < sB && pB > sT;

		if (overlap)
		{
			playerLives--;
			playerDead = true;
			playerRespawnTimer = 60;
			playerscore-=50;
			return;
		}
	}
}

int main()
{

	RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Resize);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	//************************************ FOR DECLARING THE HEART SHAPE IN LIFE SYSTEM************************************* */

	Texture livechecktexture;
	if (!livechecktexture.loadFromFile("live.png"))
	{
		cerr << "Failed to load playerselection.png. Make sure the file exists in the working directory!" << endl;
		return -1;
	}

	Sprite livechecksprite;
	livechecksprite.setTexture(livechecktexture);
	livechecksprite.setScale(1.8,1.8);
	livechecksprite.setPosition(0,0);

	//************************************* ENDED HERE*************************************************************** */

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

	int player;

	bgTex.loadFromFile("./Data/bg.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0, 0);

	blockTexture.loadFromFile("./Data/block1.png");
	blockSprite.setTexture(blockTexture);

	// Music initialisation
	Music lvlMusic;

	lvlMusic.openFromFile("./Data/mus.ogg");
	lvlMusic.setVolume(2);
	lvlMusic.play();
	lvlMusic.setLoop(true);

	// player data
	float player_x = 500;
	float player_y = 150;

	int playerLives = 3;
	bool playerDead = false;
	int playerRespawnTimer = 0;
	int playerscore=0;

	float playerDefaultX = 500;
	float playerDefaultY = 150;

	const int capLimitLvl1 = 3;
	int capLimit = capLimitLvl1;

	// stack arrays
	int capEnemyID[capLimit];	// stores index (0–7 for ghost, 0–3 for skeleton)
	int capEnemyType[capLimit]; // true = ghost, false = skeleton
	int capTop = -1;			// top of stack (-1 = empty)

	float speed = 5;

	const float jumpStrength = -20; // Initial jump velocity
	const float gravity = 1;		// Gravity acceleration

	bool isJumping = false; // Track if jumping

	int playerRunIndex = 0;
	float playerRunTimer = 0;

	bool up_collide = false;
	bool left_collide = false;
	bool right_collide = false;

	Texture PlayerTexture;
	Sprite PlayerSprite;
	Texture shootBeamTexture;
	Sprite shootBeamSprite;
	Texture GhostTexture;
	Sprite GhostSprite;

	bool onGround = false;

	float offset_x = 0;
	float offset_y = 0;
	int velocityY = 0;

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

	PlayerTexture.loadFromFile("./Data/greenplayer1.png");
	PlayerSprite.setTexture(PlayerTexture);
	PlayerSprite.setScale(float(PlayerWidth) / PlayerTexture.getSize().x, float(PlayerHeight) / PlayerTexture.getSize().y);
	PlayerSprite.setPosition(player_x, player_y);

	Texture playerRunFrames[3];
	playerRunFrames[0].loadFromFile("./Data/greenplayer2.png");
	playerRunFrames[1].loadFromFile("./Data/greenplayer3.png");
	playerRunFrames[2].loadFromFile("./Data/greenplayer4.png");

	Texture ghostThrownFrames[8];
	ghostThrownFrames[0].loadFromFile("./Data/ghost20.png");
	ghostThrownFrames[1].loadFromFile("./Data/ghost21.png");
	ghostThrownFrames[2].loadFromFile("./Data/ghost22.png");
	ghostThrownFrames[3].loadFromFile("./Data/ghost23.png");
	ghostThrownFrames[4].loadFromFile("./Data/ghost24.png");
	ghostThrownFrames[5].loadFromFile("./Data/ghost25.png");
	ghostThrownFrames[6].loadFromFile("./Data/ghost26.png");
	ghostThrownFrames[7].loadFromFile("./Data/ghost27.png");

	Texture skThrownFrames[9];
	skThrownFrames[0].loadFromFile("./Data/skeleton25.png");
	skThrownFrames[1].loadFromFile("./Data/skeleton26.png");
	skThrownFrames[2].loadFromFile("./Data/skeleton27.png");
	skThrownFrames[3].loadFromFile("./Data/skeleton28.png");
	skThrownFrames[4].loadFromFile("./Data/skeleton29.png");
	skThrownFrames[5].loadFromFile("./Data/skeleton30.png");
	skThrownFrames[6].loadFromFile("./Data/skeleton31.png");
	skThrownFrames[7].loadFromFile("./Data/skeleton32.png");
	skThrownFrames[8].loadFromFile("./Data/skeleton33.png");


	Text livesText;				//****************************************ADD THIS FOR LIFE SYSTEM TEXT*************************
	Text scoreText;				//****************************************ADD THIS FOR SCORE SYSTEM TEXT*************************

	const int shootBeamWidth = 71;
	const int shootBeamHeight = 300;
	bool shootBeamShow = false;

	shootBeamTexture.loadFromFile("./Data/shootBeam1.png");
	shootBeamSprite.setTexture(shootBeamTexture);
	shootBeamSprite.setPosition(player_x - shootBeamWidth, player_y + 43);

	const int ghostCount = 8;
	const int skeletonCount = 4;

	Texture ghostTexture;
	Texture skeletonTexture;

	Sprite ghostSpriteArr[ghostCount];
	Sprite skeletonSpriteArr[skeletonCount];

	float ghostX[ghostCount];
	float ghostY[ghostCount];
	float skeletonX[skeletonCount];
	float skeletonY[skeletonCount];

	int ghostThrowAnimIndex[ghostCount] = {0};
	float ghostThrowAnimTimer[ghostCount] = {0};

	int skThrownAnimIndex[skeletonCount] = {0};
	float skThrownAnimTimer[skeletonCount] = {0};

	float ghostSpeed = 0.8f;

	bool ghostRight[ghostCount];			  // ghost direction
	bool ghostCaptured[ghostCount] = {false}; // ghost is inside vacuum
	bool ghostDead[ghostCount] = {false};	  // ghost killed after shooting
	bool ghostInVac[ghostCount] = {false};
	bool ghostOnGround[ghostCount] = {true};
	int ghostThrownVerDir[ghostCount] = {0};

	int playerCapturedCount = 0; // how many enemies inside bag

	bool skeletonRight[skeletonCount];
	bool skeletonCaptured[skeletonCount] = {false};
	bool skeletonDead[skeletonCount] = {false};
	bool skeletonInVac[skeletonCount] = {false};

	ghostTexture.loadFromFile("./Data/ghost1.png");
	skeletonTexture.loadFromFile("./Data/skeleton1.png");

	float ghostWidth = ghostTexture.getSize().x * 1.5;
	float ghostHeight = ghostTexture.getSize().y * 1.1;

	float skWidth = skeletonTexture.getSize().x * 1.0;
	float skHeight = skeletonTexture.getSize().y * 1.1;

	int ghostRandom[ghostCount] = {0};
	int ghostVelocityY[ghostCount] = {0};

	int skeletonVelocityY[skeletonCount];
	bool skeletonOnGround[skeletonCount];
	int skeletonCooldown[skeletonCount] = {0};
	int skeletonThrownVerDir[skeletonCount] = {0};

	bool ghostThrown[ghostCount] = {false};
	int ghostThrowDir[ghostCount] = {0}; // 0 none, 1 up, 2 right, 3 down, 4 left

	bool skeletonThrown[skeletonCount] = {false};
	int skeletonThrowDir[skeletonCount] = {0};

		Font font;
	if (!font.loadFromFile("Aileron.ttf"))
	{
		cout << "Failed to load arial.ttf\n";
		return -1;
	}

	

	// creating level array
	lvl = new char *[height];
	for (int i = 0; i < height; i += 1)
	{
		lvl[i] = new char[width];
	}

	for (int i = 0; i < width; i++)
	{
		// TOP platform

		if (i == 0 || i == 1 || i == 17 || i == 16)
		{
			lvl[4][i] = ' ';
		}
		else
			lvl[4][i] = '#';

		// Second platform

		if (i == 3 || i == 4 || i == 5 || i == 10 || i == 11 || i == 12)
			lvl[7][i] = ' ';
		else
			lvl[7][i] = '#';

		// Third platform

		if (i == 5 || i == 7 || i == 6 || i == 11 || i == 12 || i == 13)
			lvl[10][i] = ' ';
		else
			lvl[10][i] = '#';

		// Last platform

		lvl[13][i] = '#';
	}

	initGhosts(ghostTexture, ghostSpriteArr, ghostX, ghostY, ghostRight, ghostCount, ghostWidth, ghostHeight, ghostOnGround);
	initSkeletons(skeletonTexture, skeletonSpriteArr,
				  skeletonX, skeletonY, skeletonRight,
				  skeletonVelocityY, skeletonOnGround,
				  skeletonCooldown,
				  skeletonCount, skWidth, skHeight);

	bool facing_right = false; // player facing direction
	bool playerIsRunning = false;
	float pX = 0;

	float movX = 0;
	Event ev;
	// main loop
	while (window.isOpen())
	{
		movX = 0;
		playerIsRunning = false;

		updateGhosts(lvl, ghostSpriteArr, ghostTexture, ghostX, ghostY, ghostRight, ghostThrown, ghostRandom,
					 ghostCount, ghostSpeed, ghostWidth, ghostHeight, cell_size, screen_x, ghostDead, ghostCaptured, ghostInVac);

		updateSkeletons(lvl,
						skeletonSpriteArr, skeletonTexture, skeletonX, skeletonY,
						skeletonRight, skeletonThrown, skeletonVelocityY,
						skeletonOnGround, skeletonCooldown,
						skeletonCount,
						1.0f, jumpStrength, gravity,
						terminal_Velocity,
						cell_size, screen_x,
						skWidth, skHeight, skeletonDead, skeletonCaptured, skeletonInVac);

		int prevPlayerLives = playerLives;

		updatePlayerLifeSystem(
			player_x, player_y, facing_right, PlayerWidth, PlayerHeight, playerDefaultX, playerDefaultY, playerLives,
			playerDead, playerRespawnTimer, ghostX, ghostY, ghostRight, ghostDead, ghostCaptured, ghostInVac, ghostThrown, ghostCount,
			ghostWidth, ghostHeight, skeletonX, skeletonY, skeletonRight, skeletonDead, skeletonCaptured, skeletonInVac,
			skeletonThrown, skeletonCount, skWidth, skHeight,playerscore);

		if (playerLives < prevPlayerLives)
		{
			for (int i = capLimit - 1; i >= 0; i--)
			{
				int newThrowDir = facing_right ? 2 : 4;
				if (capTop >= 0)
				{
					int id = capEnemyID[capTop];
					int type = capEnemyType[capTop];

					if (type == 1)
						ghostThrowDir[id] = newThrowDir;
					else if (type == 2)
						skeletonThrowDir[id] = newThrowDir;
				}

				// now actually throw the enemy
				throwLastCapturedEnemy(
					player_x, player_y, PlayerWidth, PlayerHeight, facing_right,
					ghostX, ghostY, ghostThrown, ghostThrowDir, ghostCaptured, ghostOnGround, ghostVelocityY,
					ghostThrowAnimIndex, ghostThrowAnimTimer, ghostThrownVerDir, ghostHeight, ghostWidth,
					skeletonX, skeletonY, skeletonThrown, skeletonThrowDir, skeletonCaptured, skeletonOnGround,
					skeletonVelocityY, skeletonThrownVerDir, skWidth, skHeight, capEnemyID, capEnemyType, capTop, playerCapturedCount);
			}
		}

		//******************************************************ADD THESE LINES FOR TEXT OF LIFE SYSTEM AND SCORE SYSTEM ******************************************/

 // FOR PLAYER LIVE TEXT

livesText.setFont(font);
livesText.setCharacterSize(35);
livesText.setFillColor(sf::Color::White);
livesText.setPosition(70, 10);
livesText.setString( "X" + std::to_string(playerLives));

//FOR PLAYERSCORE TEXT

if(playerscore<0)
{
	playerscore=0;
}
scoreText.setFont(font);
scoreText.setCharacterSize(35);
scoreText.setFillColor(sf::Color::White);
scoreText.setPosition(980, 10);
scoreText.setString( "X" + std::to_string(playerscore));

//************************************************************* THIS WHOLE PIECE OF CODE******************************************* */

		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed)
			{
				window.close();
			}
		}

		if (!playerDead)
		{
			// presing escape to close
			if (Keyboard::isKeyPressed(Keyboard::Escape))
			{
				window.close();
			}
			if (Keyboard::isKeyPressed(Keyboard::Left))
			{
				if (facing_right) // changing player direction
				{
					PlayerSprite.setScale(float(PlayerWidth) / PlayerTexture.getSize().x, float(PlayerHeight) / PlayerTexture.getSize().y);
					player_x -= PlayerWidth;
					facing_right = false;
				}
				else if (!(player_x <= 0)) // handeling left movement
				{
					playerIsRunning = true;
					movX = -speed;
					top_left = lvl[int(player_y) / cell_size][int(player_x + movX) / cell_size];
					left_mid = lvl[int(player_y + PlayerHeight / 2) / cell_size][int(player_x + movX) / cell_size];
					bottom_left = lvl[int(player_y + PlayerHeight) / cell_size][int(player_x + movX) / cell_size];
					if (!(top_left == '#' || left_mid == '#' || bottom_left == '#'))
					{
						player_x += movX;
					}
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::Right))
			{
				if (!facing_right) // changing player direction
				{
					PlayerSprite.setScale(-float(PlayerWidth) / PlayerTexture.getSize().x, float(PlayerHeight) / PlayerTexture.getSize().y);
					player_x += PlayerWidth;
					facing_right = true;
				}
				else if (!(player_x >= screen_x)) // handeling right movement
				{
					playerIsRunning = true;
					movX = speed;
					top_right = lvl[int(player_y) / cell_size][int(player_x + movX) / cell_size];
					right_mid = lvl[int(player_y + PlayerHeight / 2) / cell_size][int(player_x + movX) / cell_size];
					bottom_right = lvl[int(player_y + PlayerHeight) / cell_size][int(player_x + movX) / cell_size];
					if (!(top_right == '#' || right_mid == '#' || bottom_right == '#'))
					{
						player_x += movX;
					}
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::Up)) // ADDED THE JUMP FUNCTION
			{
				if (onGround)
				{
					velocityY = jumpStrength;
					onGround = false;
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::Down))
			{
				if (onGround && !(player_y + PlayerHeight >= 896))
				{
					player_y += 64;
				}
			}
			if (Keyboard::isKeyPressed(Keyboard::Space))
			{
				shootBeamShow = true;
			}
			else
			{
				shootBeamShow = false;
			}

			static bool zWasPressed = false;

			bool zNow = Keyboard::isKeyPressed(Keyboard::Z);

			if (zNow && !zWasPressed)
			{
				int newThrowDir = 0; // default forward

				if (Keyboard::isKeyPressed(Keyboard::W))
					newThrowDir = 1;
				else if (Keyboard::isKeyPressed(Keyboard::D))
					newThrowDir = 2;
				else if (Keyboard::isKeyPressed(Keyboard::S))
					newThrowDir = 3;
				else if (Keyboard::isKeyPressed(Keyboard::A))
					newThrowDir = 4;
				else
					newThrowDir = facing_right ? 2 : 4;
				if (capTop >= 0)
				{
					int id = capEnemyID[capTop];
					int type = capEnemyType[capTop];

					if (type == 1)
						ghostThrowDir[id] = newThrowDir;
					else if (type == 2)
						skeletonThrowDir[id] = newThrowDir;
				}

				// now actually throw the enemy
				throwLastCapturedEnemy(
					player_x, player_y, PlayerWidth, PlayerHeight, facing_right,
					ghostX, ghostY, ghostThrown, ghostThrowDir, ghostCaptured, ghostOnGround, ghostVelocityY,
					ghostThrowAnimIndex, ghostThrowAnimTimer, ghostThrownVerDir, ghostHeight, ghostWidth,
					skeletonX, skeletonY, skeletonThrown, skeletonThrowDir, skeletonCaptured, skeletonOnGround,
					skeletonVelocityY, skeletonThrownVerDir, skWidth, skHeight, capEnemyID, capEnemyType, capTop,
					playerCapturedCount);
			}

			zWasPressed = zNow;
		}

		if (playerCapturedCount >= capLimit)
		{
			for (int i = capLimit - 1; i >= 0; i--)
			{
				int newThrowDir = facing_right ? 2 : 4;
				if (capTop >= 0)
				{
					int id = capEnemyID[capTop];
					int type = capEnemyType[capTop];

					if (type == 1)
						ghostThrowDir[id] = newThrowDir;
					else if (type == 2)
						skeletonThrowDir[id] = newThrowDir;
				}

				// now actually throw the enemy
				throwLastCapturedEnemy(
					player_x, player_y, PlayerWidth, PlayerHeight, facing_right,
					ghostX, ghostY, ghostThrown, ghostThrowDir, ghostCaptured, ghostOnGround, ghostVelocityY,
					ghostThrowAnimIndex, ghostThrowAnimTimer, ghostThrownVerDir, ghostHeight, ghostWidth,
					skeletonX, skeletonY, skeletonThrown, skeletonThrowDir, skeletonCaptured, skeletonOnGround,
					skeletonVelocityY, skeletonThrownVerDir, skWidth, skHeight, capEnemyID, capEnemyType, capTop, playerCapturedCount);
			}
		}

		pX = facing_right ? player_x - PlayerWidth : player_x; // player x for player_gravity func bcz after scaling x on -ve makes player x on the other side

		window.clear();

		display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);

		applyVerticalPhysics(
			lvl, pX, player_y, velocityY, onGround,
			gravity, terminal_Velocity, PlayerWidth, PlayerHeight,
			cell_size, screen_x, screen_y);

		PlayerSprite.setPosition(player_x, player_y);
		if (facing_right && shootBeamShow)
		{
			shootBeamSprite.setScale(-1, 1);
			shootBeamSprite.setPosition(player_x + shootBeamWidth, player_y + 43);
			window.draw(shootBeamSprite);
			updateGhostSuction(player_x, player_y, facing_right, lvl,
							   ghostX, ghostY,
							   ghostCaptured, ghostDead, ghostThrown,
							   ghostInVac,
							   ghostRight,
							   playerCapturedCount,
							   ghostCount,
							   ghostWidth, ghostHeight,
							   cell_size, capTop, capEnemyID, capEnemyType,playerscore);

			updateSkeletonSuction(player_x, player_y, lvl,
								  skeletonX, skeletonY,
								  skeletonCaptured, skeletonDead, skeletonThrown,
								  playerCapturedCount,
								  skeletonCount,
								  skWidth, skHeight,
								  cell_size, skeletonInVac, skeletonRight, facing_right, capTop, capEnemyID, capEnemyType,playerscore);
		}
		else if (!facing_right && shootBeamShow)
		{
			shootBeamSprite.setScale(1, 1);
			shootBeamSprite.setPosition(player_x - shootBeamWidth, player_y + 43);
			window.draw(shootBeamSprite);

			updateGhostSuction(player_x, player_y, facing_right, lvl,
							   ghostX, ghostY,
							   ghostCaptured, ghostDead, ghostThrown,
							   ghostInVac,
							   ghostRight,
							   playerCapturedCount,
							   ghostCount,
							   ghostWidth, ghostHeight,
							   cell_size, capTop, capEnemyID, capEnemyType,playerscore);

			updateSkeletonSuction(player_x, player_y, lvl,
								  skeletonX, skeletonY,
								  skeletonCaptured, skeletonDead, skeletonThrown,
								  playerCapturedCount,
								  skeletonCount,
								  skWidth, skHeight,
								  cell_size, skeletonInVac, skeletonRight, facing_right, capTop, capEnemyID, capEnemyType,playerscore);
		}

		if (playerIsRunning)
		{
			animateCharacter(PlayerSprite, playerRunFrames, 3, playerRunIndex, playerRunTimer,
							 0.10f, facing_right, float(PlayerWidth) / (playerRunFrames[playerRunIndex].getSize().x),
							 float(PlayerHeight) / (playerRunFrames[playerRunIndex].getSize().y));
		}
		else
		{
			PlayerSprite.setTexture(PlayerTexture);
			PlayerSprite.setScale(((facing_right) ? (-float(PlayerWidth) / (PlayerTexture.getSize().x))
												  : (float(PlayerWidth) / (PlayerTexture.getSize().x))),
								  (float(PlayerHeight) / PlayerTexture.getSize().y));
		}

		for (int i = 0; i < ghostCount; i++)
		{
			if (!shootBeamShow && ghostInVac[i])
			{
				ghostInVac[i] = false;
			}
		}

		for (int i = 0; i < skeletonCount; i++)
		{
			if (!shootBeamShow && skeletonInVac[i])
			{
				skeletonInVac[i] = false;
			}
		}

		if (!playerDead)
			window.draw(PlayerSprite);

		updateThrownEnemies(
			lvl, screen_x, screen_y, cell_size, player_x, player_y, PlayerWidth,
			PlayerHeight, facing_right, ghostX, ghostY, ghostThrown, ghostThrowDir, ghostThrownVerDir,
			ghostCaptured, ghostOnGround, ghostVelocityY, ghostCount, ghostDead, ghostInVac,
			ghostRight, ghostWidth, ghostHeight, skeletonX, skeletonY,
			skeletonCount, skWidth, skHeight, skeletonThrown, skeletonThrowDir, skeletonThrownVerDir, skeletonInVac, skeletonCaptured, skeletonOnGround,
			skeletonVelocityY, skeletonDead,playerscore);

		for (int i = 0; i < ghostCount; i++)
		{
			if (ghostThrown[i])
			{
				animateCharacter(
					ghostSpriteArr[i],													// sprite
					ghostThrownFrames,													// 8-frame throw animation
					8,																	// frameCount
					ghostThrowAnimIndex[i],												// animation index
					ghostThrowAnimTimer[i],												// animation timer
					0.2f,																// animation speed
					ghostRight[i],														// flip direction
					ghostWidth / ghostThrownFrames[ghostThrowAnimIndex[i]].getSize().x, // widthScale
					ghostHeight / ghostThrownFrames[ghostThrowAnimIndex[i]].getSize().y // heightScale
				);
			}
		}

		for (int i = 0; i < skeletonCount; i++)
		{
			if (skeletonThrown[i])
			{
				animateCharacter(
					skeletonSpriteArr[i],										// sprite
					skThrownFrames,												// 8-frame throw animation
					9,															// frameCount
					skThrownAnimIndex[i],										// animation index
					skThrownAnimTimer[i],										// animation timer
					0.14f,														// animation speed
					skeletonRight[i],											// flip direction
					skWidth / skThrownFrames[skThrownAnimIndex[i]].getSize().x, // widthScale
					skHeight / skThrownFrames[skThrownAnimIndex[i]].getSize().y // heightScale
				);
			}
		}

		for (int i = 0; i < ghostCount; i++)
		{
			if (!ghostDead[i] && !ghostCaptured[i])
				window.draw(ghostSpriteArr[i]);
		}

		for (int i = 0; i < skeletonCount; i++)
		{
			if (!skeletonDead[i] && !skeletonCaptured[i])
				window.draw(skeletonSpriteArr[i]);
		}

		if (playerLives <= 0)
		{
			playerscore-=200;			//***************************ADDED THIS TO CHECK IF PLAYER LIFE ALL 0 THEN - 200 FROM SCORE */
			break;
		}

		//*******************************************ADDED THESE THREE FOR DRAWING THE HEART IN LIFE SYSTEM AND PLAYER LIFE LEFT AND SCORE OF PLAYER************************** */


		window.draw(livechecksprite);
		window.draw(livesText);
		window.draw(scoreText);
		

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
