#pragma once

class CFluke
{
public:
	CFluke(void);
public:
	~CFluke(void);
public:
	void fluke45_init(int address);
public:
	void fluke45_set_filter (int filter);
public:
	void fluke45_config (char *fun,int range,int trigger);
public:
	void fluke45_measure (double &measure);
private:
	int m_fluke45;
};
