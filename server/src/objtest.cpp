#include<HCNSObjectPool.hpp>

class Example_ClassA:public HCNSObjectPool<Example_ClassA,10>
{
public:
    Example_ClassA(int _a,double _b,std::string &&_c)        
        :a(_a),b(_b),c(_c){}

    virtual ~Example_ClassA(){}

private:
    int a;
    double b;
    std::string c;
};

int main()
{
    //C *p(HCNSObjectPool<C,10>::createObject(10,100,98.98,std::string("hello world")));
    std::unique_ptr<Example_ClassA> ptr(new Example_ClassA(100,98.98,std::string("hello world")));
    return 0;
}