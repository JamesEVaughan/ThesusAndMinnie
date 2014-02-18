/****************************
	This is the implementation file for the TaM_Map class. This class handles
	a single map for Theseus and Minnie.
*****************************/
#include "TaM_Map.h"

using namespace std;

// Constructors/destructors
TaM_Map::TaM_Map() {
	spaces = NULL;
	name = "";
	wallHelper = new TaM_LineList(TAM_COLOR_WALL);
}

TaM_Map::~TaM_Map() {
	// Destroy all dynamic memory
	SPC_delete();

	delete startTheseus;
	delete startMinnie;
	delete theEnd;
	delete sizeSq;
	delete wallHelper;
}

int TaM_Map::init(string mapName) {
	char *mapData = NULL;
	int errCode = 0;

	// Load the map into mapData
	if (errCode = loadMap(mapName)) {
		return errCode;
	}

	// Find the points
	if (errCode = findPoints()) {
		return errCode;
	}

	// Build the lines
	buildLines();



	return ALL_CLEAR;
}

int TaM_Map::loadMap(string name) {
	ifstream ifMap(name, ios::in | ios::binary|ios::ate);
	int errCode = 0;
	if (!ifMap.is_open()) {
		// Map won't open, not our problem
		// Make all output pointers NULL
		spaces = NULL;
		return ERR_MAP_OPEN;
	}

	// First, let's find out how big the file is
	int szMap = (int)ifMap.tellg();
	szMap -= 2;  // The first two bytes tell us the map dimensions

	// Now, let's reset the reader and pop off the map size
	ifMap.seekg(0, ios::beg);
	int width = ifMap.get();
	sizeSq = new TaM_IntVector(width, ifMap.get());

	// Sanity check
	if (szMap < (sizeSq->get1() * sizeSq->get2())) {
		// Map is bad
		return ERR_MAP_BAD;
	}

	// Now, load the rest into data
	spaces = new char[szMap];
	ifMap.read(spaces, (szMap));
	if (!ifMap.good() && szMap != ifMap.gcount()) {
		// Problem with the file
		// Delete all new objects
		delete sizeSq;
		delete[] spaces;
		if (ifMap.eof()) {
			// The map file is too short
			return ERR_MAP_EOF;
		}
		else {
			// The map file is just bad
			return ERR_MAP_BAD;
		}
	}

	// Now close up shop
	ifMap.close();
	return ALL_CLEAR;
}

int TaM_Map::findPoints() {
	// Set the position markers to null
	startTheseus = startMinnie = theEnd = NULL;
	// Just scan the whole thing, looking for each point
	char tempy;
	for (int y = 0; y < sizeSq->get2(); y++) {
		for (int x = 0; x < sizeSq->get1(); x++) {
			tempy = SPC_getSpace(x, y);
			if (tempy & START_THE) {
				if (startTheseus) {
					// Bad map
					return ERR_MAP_BAD;
				}
				startTheseus = new TaM_IntVector(x, y); 
			}
			else if (tempy & START_MIN) {
				if (startMinnie) {
					// Bad map
					return ERR_MAP_BAD;
				}
				startMinnie = new TaM_IntVector(x, y);
			}
			else if (tempy & THE_END) {
				if (theEnd) {
					// Bad map
					return ERR_MAP_BAD;
				}
				theEnd = new TaM_IntVector(x, y);
			}
		}
	}

	if (!startTheseus || !startMinnie || !theEnd) {
		// Bad map
		return ERR_MAP_BAD;
	}

	return ALL_CLEAR;
}

void TaM_Map::writeToConsole() {
	int x = sizeSq->get1();
	int y = sizeSq->get2();
	cout << "(" << x << ", " << y << ")" << endl;
	for (int i = 0; i < y; i++) {
		printf("%3X", (unsigned char)SPC_getSpace(0, i));
		for (int j = 1; j < x; j++) {
			printf(" %3X", (unsigned char)SPC_getSpace(j, i));
		}
		cout << endl;
	}

	// Do the same for wallHelper too
	wallHelper->writeToConsole();
}

/***********************
	buildLines()
	takes the information from *spaces and constructs all the lines that they imply
************************/
void TaM_Map::buildLines() {
	GLfloat lineStart[2] = {6.f, 6.f};	// Start of the line
	GLfloat lineEnd[2] = {6.f, 6.f};	// End of the line

	// Set up the offsets in both directions to center the map
	// NOTE: With an odd number of squares, the extra square will be on the right and below, respectively
	GLfloat xOffset = ((TAM_GRID_SIZE - sizeSq->get1()) / 2.f) * TAM_SQUARE_SIZE - 1;
	GLfloat yOffset = 1 - (((TAM_GRID_SIZE - sizeSq->get2()) / 2.f) * TAM_SQUARE_SIZE);

	/*********
		The algorithm evaluates each value in spaces to find the horizontal and 
		vertical lines. The main nested-for's test the north and west border for 
		each space. The for immediately folowing handles the southern and 
	**********/
	// Puposefully skip the 0th row and column, will never have a valid wall
	for (int y = 0; y < sizeSq->get2(); y++) {
		for (int x = 0; x < sizeSq->get1(); x++) {
			char spc = SPC_getSpace(x, y);

			// Don't do anything with THE_END
			if (spc & THE_END) {
				continue;
			}

			// Zero out all bits that aren't wall bits
			spc &= WALL_MASK;

			if (spc & WALL_NORTH) {
				// Start of a new line?
				if (lineStart[0] > 1.f) {
					lineStart[0] = xOffset + (TAM_SQUARE_SIZE * x);
					lineStart[1] = yOffset - (TAM_SQUARE_SIZE * y);
				}
			}
			else if (!(lineStart[0] > 1.f)) {
				// End of the line?
				lineEnd[0] = xOffset + (TAM_SQUARE_SIZE * x);
				lineEnd[1] = yOffset - (TAM_SQUARE_SIZE * y);
				
				wallHelper->addLine(lineStart[0], lineStart[1], lineEnd[0], lineEnd[1]);
				lineStart[0] = lineStart[1] = lineEnd[0] = lineEnd[1] = 6.f;
			}
		}
	}

	// Check for vertical lines
	for (int x = 0; x < sizeSq->get1(); x++) {
		for (int y = 0; y < sizeSq->get2(); y++) {
			char spc = SPC_getSpace(x, y);

			// Don't do anything with THE_END
			if (spc & THE_END) {
				continue;
			}

			// Zero out all bits that aren't wall bits
			spc &= WALL_MASK;

			if ((spc & WALL_WEST)) {
				// Start of a new line?
				if (lineStart[0] > 1.f) {
					lineStart[0] = xOffset + (TAM_SQUARE_SIZE * x);
					lineStart[1] = yOffset - (TAM_SQUARE_SIZE * y);
				}
			}
			else if (!(lineStart[0] > 1.f)) {
				// End of the line?
				lineEnd[0] = xOffset + (TAM_SQUARE_SIZE * x);
				lineEnd[1] = yOffset - (TAM_SQUARE_SIZE * y);
					
				wallHelper->addLine(lineStart[0], lineStart[1], lineEnd[0], lineEnd[1]);
				lineStart[0] = lineStart[1] = lineEnd[0] = lineEnd[1] = 6.f;
			}
		}
	}
	// Finally, take care of the last playable row...
	for (int x = 0; x < sizeSq->get1(); x++) {
		int y = sizeSq->get2()-2;

		char spc = SPC_getSpace(x, y);
		// Zero out all bits that aren't wall bits
		spc &= WALL_MASK;

		if ((spc & WALL_SOUTH)) {
			// Start of a new line?
			if (lineStart[0] > 1.f) {
				lineStart[0] = xOffset + (TAM_SQUARE_SIZE * x);
				lineStart[1] = yOffset - (TAM_SQUARE_SIZE * (y+1));
			}
		}
		else if (!(lineStart[0] > 1.f)) {
			// End of the line?
			lineEnd[0] = xOffset + (TAM_SQUARE_SIZE * x);
			lineEnd[1] = yOffset - (TAM_SQUARE_SIZE * (y+1));
					
			wallHelper->addLine(lineStart[0], lineStart[1], lineEnd[0], lineEnd[1]);
			lineStart[0] = lineStart[1] = lineEnd[0] = lineEnd[1] = 6.f;
		}

	}
	// ...and column
	for (int y = 0; y < sizeSq->get2(); y++) {
		int x = sizeSq->get1()-2;

		char spc = SPC_getSpace(x, y);
		// Zero out all bits that aren't wall bits
		spc &= WALL_MASK;

		if (spc & WALL_EAST) {
			// Start of a new line?
			if (lineStart[0] > 1.f) {
				lineStart[0] = xOffset + (TAM_SQUARE_SIZE * (x+1));
				lineStart[1] = yOffset - (TAM_SQUARE_SIZE * y);
			}
		}
		else if (!(lineStart[0] > 1.f)) {
			// End of the line?
			lineEnd[0] = xOffset + (TAM_SQUARE_SIZE * (x+1));
			lineEnd[1] = yOffset - (TAM_SQUARE_SIZE * y);
					
			wallHelper->addLine(lineStart[0], lineStart[1], lineEnd[0], lineEnd[1]);
			lineStart[0] = lineStart[1] = lineEnd[0] = lineEnd[1] = 6.f;
		}
	}

	// Build the lines for the end square
	char end = SPC_getSpace(theEnd->get1(), theEnd->get2());
	if (end & WALL_NORTH) {
		wallHelper->addLine(
			xOffset + (theEnd->get1() * TAM_SQUARE_SIZE), 
			yOffset - (theEnd->get2() * TAM_SQUARE_SIZE),
			xOffset + ((theEnd->get1()+1) * TAM_SQUARE_SIZE), 
			yOffset - (theEnd->get2() * TAM_SQUARE_SIZE));
	}

	if (end & WALL_WEST) {
		wallHelper->addLine(
			xOffset + (theEnd->get1() * TAM_SQUARE_SIZE), 
			yOffset - (theEnd->get2() * TAM_SQUARE_SIZE),
			xOffset + (theEnd->get1() * TAM_SQUARE_SIZE), 
			yOffset - ((theEnd->get2()+1) * TAM_SQUARE_SIZE));
	}

	if (end & WALL_EAST) {
		wallHelper->addLine(
			xOffset + ((theEnd->get1()+1) * TAM_SQUARE_SIZE), 
			yOffset - (theEnd->get2() * TAM_SQUARE_SIZE),
			xOffset + ((theEnd->get1()+1) * TAM_SQUARE_SIZE), 
			yOffset - ((theEnd->get2()+1) * TAM_SQUARE_SIZE));
	}

	if (end & WALL_SOUTH) {
		wallHelper->addLine(
			xOffset + (theEnd->get1() * TAM_SQUARE_SIZE), 
			yOffset - ((theEnd->get2()+1) * TAM_SQUARE_SIZE),
			xOffset + ((theEnd->get1()+1) * TAM_SQUARE_SIZE), 
			yOffset - ((theEnd->get2()+1) * TAM_SQUARE_SIZE));
	}
}

void TaM_Map::draw() {
	// For now, just draw wallHelper and see what happens
	wallHelper->drawLines();
}

TaM_IntVector TaM_Map::getTheseus() {
	return *startTheseus;
}

TaM_IntVector TaM_Map::getMinnie() {
	return *startMinnie;
}

TaM_IntVector TaM_Map::getTheEnd() {
	return *theEnd;
}

TaM_IntVector TaM_Map::getSize() {
	return *sizeSq;
}