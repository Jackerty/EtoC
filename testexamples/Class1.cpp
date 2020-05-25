/*****************************************************
* This file is to test class to structure compiling  *
* when all the class definitions are at same place.  *
* Codng style is terrible on purpose for testing     *
* testing reasons.                                   *
*****************************************************/
#include						<iostream>
#include<cstdlib>
#include<termios.h>
#include   <sys/ioctl.h>
#include <unistd.h>
#include<csignal>
#include<string>

/*
* Back up terminal name can be
* given by gcc's -D option.
* If it isn't given then macro
* BACKUP_TERM is defined here.
* Backup term is used if for some
* reason TERM eviromental variable
* isn't defined.
*/
#ifndef BACKUP_TERM
#	define BACKUP_TERM xterm
#endif
/*
* This just macro which signals for programmer
* parameter is ignored as doesn't effect anything.
*/
#define IGNORED 0

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

struct Deployment
{
	Deployment *next;
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
	virtual Deployment** deploy();
	virtual Unit* get_towers();
	virtual Unit* get_soldiers();
	void recieve_funds(int funds);
	bool salary_paid(Unit* u);

protected:
	int id;
	Unit* towers;
	Unit* army;
	Deployment *depoymentlist;
	long money;
private:


};
class Player:public Commander{

};
class AI:public Commander{

};

namespace CSI{
	/*
	This is CSI color class.
	*/
	class CSIcolor{
	private:
		std::string color;
	public:
		CSIcolor(const char *red,const char *green,const char *blue){
			color.resize(3+1+3+1+3);
			color.append(red);
			color.append(";");
			color.append(green);
			color.append(";");
			color.append(blue);
		}
		const std::string *getColor() const{
			return &color;
		}
	};
	/*
	* Some constant colors.
	*/
	static const CSIcolor BLUE("000","000","255");
	static const CSIcolor RED("255","000","000");
	static const CSIcolor GREEN("000","255","000");
};
/*
Clear the terminal screen.
*/
void clearTerminal(){
	std::cout<<"\e[1;1H\e[2J"<<std::endl;
}
/*
Get input state.
*/
int getInputState(){
	uint32_t state;
	std::cin>>state;
	return state;
}
/*
Terminal state and size structures.
*/
struct termios TermBuffer;
struct winsize TermSize;
/*
Have two frames buffers so that separete update
and render threads could be done.
*/
std::string frames[2];
short currentFrame=0;
/*
Update terminal size
*/
void updateTerminalSize(int ignore){
	// Query the size.
	if(ioctl(STDIN_FILENO,TIOCGWINSZ,(void*)&TermSize)==0){
		// Resize unused frame.
		// Have ws_row times ws_col characters which may have
		// all different color. To get the color CSI code must
		// be used. For 24bit (3 channel) white foreground color
		// code is "\e[38;2;255;255;255m" for background same is
		// "\e[48;2;255;255;255m". This would be 2*19 bytes for color
		// plus one for letter.
		unsigned int allocation=TermSize.ws_row*TermSize.ws_col*(2*19+1);
		if(allocation>frames[0].capacity()){
			frames[0].resize(allocation);
			frames[1].resize(allocation);
			for(std::string frame:frames){
				frame.assign("\e[38;2;255;255;255m\e[48;2;255;255;255m");
			}
		}
	}
	else std::cerr<<"Error on ioctl TIOCGWINSZ!"<<std::endl;
}
/*
Write character to current frame buffer.
*/
void writeCharToFrame(const std::string *backcolor,const std::string *forecolor,const char c,const short row,const short col){
	// Get vector which is the frame we are changing. 
	std::string &frame=frames[currentFrame];
	// Calculate the index from row and col.
	// reminder that every pixels has 2*19+1
	// bytes. Background \e[38;2;255;255;255m
	// and foreground \e[48;2;255;255;255m.
	const int index=(row*TermSize.ws_col+col)*(2*19+1);
	// Set backcolor and forecolor.
	for(int i=0;i<11;i++){
		frame[index+7+i]=(*backcolor)[i];
		frame[index+26+i]=(*forecolor)[i];
	}
	// Set the character.
	frame[index+2*19]=c;
}
	/*
Swap framebuffers.
*/
void swapFrame(){
	std::cout<<frames[currentFrame];
}
/*
Main function for this test example.
State machine interface is defined
here. Every choice in the game moves
time.
*/
int main(){
	// If terminal isn't terminal just stop.
	// Program needs screen to render.
	if(isatty(STDIN_FILENO)){
		clearTerminal();
		updateTerminalSize(IGNORED);
		
		// Set up signal handler for terminal resize.
		if(signal(SIGWINCH,updateTerminalSize)!=SIG_ERR){
			writeCharToFrame(CSI::GREEN.getColor(),CSI::RED.getColor(),'H',0,0);
			swapFrame();
			while(1){
				switch(getInputState()){
				case 1:
					goto EXIT;
				case 0:
				case 2:
				case 3:
					break;
				}
			}
			EXIT:
			// Before exiting make sure that terminal is black text and white background.
			// So that terminal doesn't have odd coloring after program is run.
			// TODO: Query terminal colors before program starts?
			clearTerminal();
			std::cout<<"\e[38;2;0;0;0m\e[48;2;255;255;255m"<<std::endl;
		}
		else std::cerr<<"ERROR: signal SIGWINCH"<<std::endl;
	}
	else std::cerr<<"Stdin has to be terminal device."<<std::endl;

	return 0;
}
