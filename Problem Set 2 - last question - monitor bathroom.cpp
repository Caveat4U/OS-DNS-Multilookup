monitor restroom 
{
	enum {MALE_WAITING, FEMALE_WAITING} bathroom_gender[N];
	condition self[N];


	void exit(int i)
	{
		state[i] = !self[i];
	}

	void use(int i)
	{
		state[i] = self[i];
	}
	void check(int i)
	{
		if(state[i] != self[i])
		{
			self[i].wait();
		}
	}
}