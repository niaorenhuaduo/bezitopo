/******************************************************/
/*                                                    */
/* random.h - random numbers                          */
/*                                                    */
/******************************************************/

class randm
{
public:
  randm();
  unsigned short usrandom();
  ~randm();
private:
  FILE *randfil;
};

extern randm rng;