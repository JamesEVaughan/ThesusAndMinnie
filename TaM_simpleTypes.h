/*
	This header contains a number of simple data structures for use throughout
	Theseus and Minnie.
*/

#ifndef TAM_SIMPLE_TYPES
#define TAM_SIMPLE_TYPES

/***********************TaM_IntVector BEGIN****************
Class.
Simple 2-value, int vector with basic accessor and mutator functions.
*/

class TaM_IntVector {
private:
	int val1;
	int val2;

public:
	// Constructor
	TaM_IntVector(int one, int two);
	TaM_IntVector();

	// Accessors
	int get1() {return val1;}
	int get2() {return val2;}

	// Mutators
	void set1(int new1) {val1 = new1;}
	void set2(int new2) {val2 = new2;}

	// Returns true if this == oth
	bool equalTo(TaM_IntVector oth);

	// Overloading Operators
	TaM_IntVector operator- (const TaM_IntVector &oth);
};

class TaM_FloatVector {
private:
	float val1;
	float val2;

public:
	// Constructor
	TaM_FloatVector(float one, float two);
	TaM_FloatVector();

	// Accessors
	float get1() {return val1;}
	float get2() {return val2;}

	// Mutators
	void set1(float new1) {val1 = new1;}
	void set2(float new2) {val2 = new2;}
};

#endif