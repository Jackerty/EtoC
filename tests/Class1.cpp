/*****************************************************
* This file is to test class to structure compiling  *
* when all the class definitions are at same place.  *
* More realistic sample (with modification)          *
* is taken from:                                     *
* https://github.com/Fluxilis/GereonTDAI.            *
*****************************************************/
#include						<unistd.h>
#define BUFFER_IO_SIZE 1024

class Cell;

class Unit
{
public:
	Unit(int o);
	~Unit();
	void set_signature(char sig);
	char get_signature();
	int get_owner();
	Cell* get_position();
	void set_owner(int i);
	bool is_active();
	int get_range();
	virtual bool attack_unit(Unit *target);
	void damage_unit(int damage);
	int get_health();
	void reset();
	bool is_in_range(Cell* c);
	virtual bool deploy(Cell* c);
	virtual bool score();
	virtual bool move_to(Cell* destination);
	virtual bool place_tower(Cell* c, long* money);
	void activate();
	bool has_scored();
	int* get_cost();
	void deactivate();
protected:


	int owner;
	char signature;
	Cell* position;
	int health;
	int maxHealth;
	bool canAct = true;
	int attackStrength;
	int range;
	bool freezes = false;
	bool scored = false;
	int cost;
private:


};

class Empty : public Unit
{
public:
	Empty();

};


class Tower : public Unit
{
public:
	Tower(int o);
	bool score();
	bool move_to(Cell* destination);
	bool place_tower(Cell* c, int* money);
	virtual bool attack_unit(Unit *target);

protected:
private:

};

class MissileTower : public Tower
{
public:
	MissileTower( int o);
	bool attack_unit(Unit *target);
protected:
private:

};


class RocketTower : public Tower
{
public:
	RocketTower( int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class FrostTower : public Tower
{
public:
	FrostTower( int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class Blockade : public Tower
{
public:
	Blockade( int o);
	bool attack_unit(Unit *target);
protected:
private:

};


class Soldier : public Unit
{
public:
	Soldier(int o);
	int get_speed();
	bool move_to(Cell* destination);
	bool deploy(Cell* c);
	bool score();
	virtual bool attack_unit(Unit *target);

protected:
	int speed;


private:

};

class ShieldSoldier : public Soldier
{
public:
	ShieldSoldier(int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class RifleSoldier : public Soldier
{
public:
	RifleSoldier(int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class BombSoldier : public Soldier
{
public:
	BombSoldier(int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class HorseSoldier : public Soldier
{
public:
	HorseSoldier(int o);
	bool attack_unit(Unit *target);
protected:
private:

};

class Cell{
	public:
		Cell(Unit &oc, int xpos, int ypos, Cell*** l);
		bool add_occupant(Unit &o);
		bool remove_occupant(Unit &oc);
		bool remove_tower(Unit &oc);
		void printCell();
		int get_X();
		int get_Y();
		auto find_occupant(Unit &oc);
		Unit *get_occupants();
		bool is_blocked();
		bool deploy_unit(Unit &oc);
		Unit* get_units(short owner);
		bool build_tower(Unit &oc);
		bool move_unit_to(Unit &oc, Cell* to);
		bool line_full();
	protected:

	private:
		Cell *** lane;
		Unit **occupant;
		Unit & fallback;
		int x;
		int y;
		bool blocked = false;
		void block();
		void unblock();
};

struct Depoyment
{
	Depoyment *next;
	int pos;
	int wave;
	Unit* unit;
};

class Commander
{
public:
	Commander(int id, int w, int h);
	int width;
	int hight;
	int get_id();
	virtual void build_turrets(Cell *** &lane);
	int update_towers();
	virtual void recrute_soldiers();
	virtual Depoyment** deploy();
	virtual Unit* get_towers();
	virtual Unit* get_soldiers();
	void recieve_funds(int funds);
	bool salary_paid(UnitAI* u);

protected:
	int id;
	Unit* towers;
	Unit* army;
	Depoyment *depoymentlist;
	long money;
private:


};

int main(){

	char inputbuffer[BUFFER_IO_SIZE];
	int inputpoint=0;
	int n;
	while((n=read(STDIN_FILENO,inputbuffer,BUFFER_IO_SIZE))>0){
		switch(inputbuffer[inputpoint]){
		case '1':
		case '2':
		case '3':
			break;
		}
	}

	return 0;
}
