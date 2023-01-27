#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#pragma once
static std::unordered_map<std::string, int> const_val;//对于常量，直接映射到其值
static std::unordered_map<std::string, int> var_type;//确定某一变量的类型（常量 or 变量？）
static std::unordered_map<std::string, int> func_ret;
static int nowww=1;
static int dep=0,nowdep=0,bl_dep=0,nowbl=0,glo_var=0,arr_val=0;
//static std::vector<int> f;
static int f[10005],iff[10005],be_end_dep[10005],blf[10005],be_end_bl[10005],whf[10005];
static int if_cnt=0,now_if=0,wh_cnt=0,now_wh=0;
static std::vector<int> array_dim;
static int filled_sum=0,brace_dep=0,be_func_para;
static std::unordered_map <std::string,int> array_siz; 
class BaseAST
{
    public:
        virtual ~BaseAST()=default;
        virtual void Dump() = 0;
        virtual int Calc() =0;
        virtual void Show()
        {
            return;
        }
        virtual std::vector<int> Para()
        {
            std::vector<int> v;
            return v;
        }
};

class SinCompUnit:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> func_def;
        void Dump()override
        {
            func_def->Dump();
        }
        int Calc()override
        {
            return 23;
        }
};

class MulCompUnit:public BaseAST
{
    public:
        std::unique_ptr<BaseAST>sin_comp_unit;
        std::unique_ptr<BaseAST>mul_comp_unit;
        void Dump()override
        {
            sin_comp_unit->Dump();
            mul_comp_unit->Dump();
        }   
        int Calc()override
        {
            return 22;
        }
};

class CompUnit:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> comp_unit;
    
        void Dump() override 
        {
            //std::cout << "CompUnitAST { ";
            comp_unit->Dump();
            //std::cout << " }";
        }
        int Calc() override{ return 13; }
        //~CompUnit(){ return 0; }
};

class Compunit:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> compunit;
        void Dump()override
        {
            compunit->Dump();
        }
        int Calc()override
        {
            return 30;
        }
};

class FuncDef:public BaseAST
{
    public:
        std::string func_type;
        std::string ident;
        std::unique_ptr<BaseAST> func_para;
        std::unique_ptr<BaseAST> block;
        void Dump() override 
        {
            std::cout << "fun ";
            std::cout<<"@"<<ident<<"( ";
            if(func_para)func_para->Dump();
            std::cout<<")";
            if(func_type=="int")std::cout<<": i32";
            std::cout << "{ "<<std::endl;
            std::cout<<"%entry"<<bl_dep<<":"<<std::endl;
            
            dep++;
            f[dep]=nowdep;
            nowdep=dep;

            if(func_para)func_para->Show();

            bl_dep++;
            blf[bl_dep]=nowbl;
            nowbl=bl_dep;
            func_ret[ident]=(func_type=="int");
            
            block->Dump();
            

            if(!be_end_bl[nowbl]&&!be_end_dep[nowdep])
            {
                if(func_ret[ident])std::cout<<"    ret 0"<<std::endl;
                else std::cout<<"   ret"<<std::endl;
            }

            nowbl=blf[nowbl];
            nowdep=blf[nowdep];
            std::cout << "}"<<std::endl;
        }
        int Calc() override{ return 12; }
        //~FuncDef(){ return 0; }
};

class FuncParas:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> func_para;
        void Dump()override
        {
            func_para->Dump();
        }
        int Calc()override
        {
            return 24;
        }
        void Show()override
        {
            func_para->Show();
        }
};

class SinFuncPara:public BaseAST
{
    public:
        std::string IDENT;
        void Dump()override
        {
            std::cout<<"@"<<IDENT<<":i32";
        }
        int Calc()override
        {
            return 25;
        }
        void Show()override
        {
            std::cout<<"    @"<<"COMPILER__"+IDENT+"_"+std::to_string(nowdep)<<"= alloc i32"<<std::endl;
            std::cout<<"    store @"<<IDENT<<", @"<<"COMPILER__"+IDENT+"_"+std::to_string(nowdep)<<std::endl;
            var_type["COMPILER__"+IDENT+"_"+std::to_string(nowdep)]=1;
        }
};

class MulFuncPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_func_para;
        std::unique_ptr<BaseAST> mul_func_para;
        void Dump()override
        {
            sin_func_para->Dump();
            std::cout<<",";
            mul_func_para->Dump();
        }
        int Calc()override
        {
            return 26;
        }
        void Show()override
        {
            sin_func_para->Show();
            mul_func_para->Show();
        }
};

class ArrayFuncPara:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> siz;
        std::vector<int> size;
        void Dump()override
        {
            size=siz->Para();
            if(size.size()==0)
            {
                std::cout<<"@"<<IDENT<<": *i32";
            }else
            {
                std::cout<<"@"<<IDENT<<": *";
                for(int i=0;i<size.size();i++)std::cout<<"[";
                std::cout<<"i32, ";
                for(int i=size.size()-1;i>=0;i--)
                {
                    std::cout<<size[i]<<"]";
                    if(i!=0)std::cout<<",";
                }
            }
        }
        int Calc()override
        {
            return 51;
        }
        void Show()override
        {
            std::cout<<"    @"<<"COMPILER__"+IDENT+"_"+std::to_string(nowdep)<<"= alloc ";
            std::cout<<"*";
            for(int i=0;i<size.size();i++)std::cout<<"[";
            if(size.size())std::cout<<"i32, ";
            else std::cout<<"i32";
            for(int i=size.size()-1;i>=0;i--)
            {
                std::cout<<size[i]<<"]";
                if(i!=0)std::cout<<",";
            }
            std::cout<<std::endl;
            std::cout<<"    store @"<<IDENT<<", @"<<"COMPILER__"+IDENT+"_"+std::to_string(nowdep)<<std::endl;
            var_type["COMPILER__"+IDENT+"_"+std::to_string(nowdep)]=3;
            array_siz["COMPILER__"+IDENT+"_"+std::to_string(nowdep)]=size.size()+1;
        }
};

class ArrayParaSize:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> array_para_size;
        std::vector<int> size;
        void Dump()override
        {
            if(array_para_size)array_para_size->Dump();
        }
        int Calc()override
        {
            return 52;
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret;
            if(array_para_size)ret=array_para_size->Para();
            return ret;
        }
};

class Block:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> block;
        int typ;
        void Dump() override 
        {
            if(typ==0)
            {
                dep++;
                f[dep]=nowdep;
                nowdep=dep;
                block->Dump();
                /*if(!be_end_bl[nowbl]&&!be_end_dep[nowdep])
                {
                    std::cout<<"ret 0"<<std::endl;
                }*/
                nowdep=f[nowdep];
            }
        }
        int Calc() override{ return 10; }
        //~Block(){ return 0; }
};

class BlockItem:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> block_item;
        void Dump()override
        {
            block_item->Dump();
        }
        int Calc() override{ return 9; }
};

class MulBlockItem:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_item;
        std::unique_ptr<BaseAST> mul_item;
        void Dump()override
        {
            sin_item->Dump();
            mul_item->Dump();
        }
        int Calc() override{ return 8; }
};

class SinBlockItem:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_block_item;
        void Dump()override
        {
            if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
            sin_block_item->Dump();
        }
        int Calc() override{ return 7; }
};

class Stmt:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> lval;
        std::unique_ptr<BaseAST> exp;
        int typ;
        void Dump() override 
        {
            //std::cout<<nowdep<<" "<<nowbl<<std::endl;
            if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
            if(typ==0)//return
            {
                //if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
                be_end_dep[nowdep]=be_end_bl[nowbl]=1;
                exp->Dump();
                std::cout << "\tret " <<'%'<<nowww-1<<std::endl;
                
            }else if(typ==1)//赋值
            {
                exp->Dump();
                lval->Dump();
            }else if(typ==2)//空白表达式
            {
                exp->Dump();
            }else if(typ==3)//新的块
            {
                exp->Dump();
            }else if(typ==5)
            {
                exp->Dump();
            }else if(typ==6)
            {
                exp->Dump();
            }else if(typ==7)
            {
                exp->Dump();
            }else if(typ==8)//return
            {
                //if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
                be_end_dep[nowdep]=be_end_bl[nowbl]=1;
                //exp->Dump();
                std::cout << "\tret"<<std::endl;
            }
        }
        int Calc() override{ return 6; }
        //~Stmt(){ return 0; }
};

class Exp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> exp;
        void Dump() override
        {
            exp->Dump();
        }
        int Calc() override
        {
            return exp->Calc();
        }
        std::vector<int> Para()override
        {
            filled_sum++;
            std::vector<int> ret;
            exp->Dump();
            ret.push_back(nowww-1);
            return ret;
        }
};

class SinExp:public BaseAST
{
    public:
        char una_op;
        std::unique_ptr<BaseAST> una_exp;
        void Dump()override
        {
            una_exp->Dump();
            if(una_op=='-')
            {
                std::cout<<"\t"<<'%'<<nowww<<"= "<<"sub 0,"<<'%'<<nowww-1<<std::endl;
                ++nowww;
            }else if(una_op=='!')
            {
                std::cout<<"\t"<<'%'<<nowww<<"= "<<"eq 0,"<<'%'<<nowww-1<<std::endl;
                ++nowww;
            }
        }
        int Calc() override
        {
            if(una_op=='-')
                return -(una_exp->Calc());
            else if(una_op=='+')
                return una_exp->Calc();
            else 
                return !(una_exp->Calc());
        }
};

class UnaryExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> un_exp;
        void Dump() override
        {
            un_exp->Dump();
        }
        int Calc() override
        {
            return un_exp->Calc();
        }
};

class NumExp:public BaseAST
{
    public:
        int num;
        void Dump()override
        {
            std::cout<<"\t"<<'%'<<nowww<<"= add "<<"0, "<<num<<std::endl;
            ++nowww;
        }
        int Calc() override
        {
            return num;
        }
};

class PrimaryExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> p_exp;
        void Dump()override
        {
            p_exp->Dump();
        }
        int Calc() override
        {
            return p_exp->Calc();
        }
};

class MulExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> mul_exp;
        void Dump()override
        {
            mul_exp->Dump();
        }
        int Calc() override
        {
            return mul_exp->Calc();
        }
};

class MultiExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> mu_exp;
        char op;
        std::unique_ptr<BaseAST> un_exp;
        void Dump()override
        {
            mu_exp->Dump();
            int now1=nowww-1;
            un_exp->Dump();
            int now2=nowww-1;
            if(op=='*')
            {
                std::cout<<"\t"<<'%'<<nowww<<"= mul "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else if(op=='/')
            {
                std::cout<<"\t"<<'%'<<nowww<<"= div "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else
            {
                std::cout<<"\t"<<'%'<<nowww<<"= mod "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }
        }
        int Calc() override
        {
            if(op=='*')
                return (mu_exp->Calc())*(un_exp->Calc());
            else if(op=='/')
                return (mu_exp->Calc())/(un_exp->Calc());
            else
                return (mu_exp->Calc())%(un_exp->Calc());
        }
};

class AddExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> add_exp;
        void Dump()override
        {
            add_exp->Dump();
        }
        int Calc() override
        {
            return add_exp->Calc();
        }
};

class MuladdExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> ad_exp;
        char op;
        std::unique_ptr<BaseAST> mult_exp;
        void Dump()override
        {
            ad_exp->Dump();
            int now1=nowww-1;
            mult_exp->Dump();
            int now2=nowww-1;
            if(op=='+')
            {
                std::cout<<"\t"<<'%'<<nowww<<"= add "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else
            {
                std::cout<<"\t"<<'%'<<nowww<<"= sub "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }
        }
        int Calc() override
        {
            if(op=='+')
                return (ad_exp->Calc())+(mult_exp->Calc());
            else 
                return (ad_exp->Calc())-(mult_exp->Calc());
        }
};

class RelExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> rel_exp;
        void Dump()override
        {
            rel_exp->Dump();
        }
        int Calc() override
        {
            return rel_exp->Calc();
        }
};

class MulRelExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> re_exp;
        std::string op;
        std::unique_ptr<BaseAST> ad_exp;
        void Dump()override
        {
            re_exp->Dump();
            int now1=nowww-1;
            ad_exp->Dump();
            int now2=nowww-1;
            if(op=="<")
            {
                std::cout<<"\t"<<'%'<<nowww<<"= lt "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else if(op==">")
            {
                std::cout<<"\t"<<'%'<<nowww<<"= gt "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else if(op=="<=")
            {
                std::cout<<"\t"<<'%'<<nowww<<"= le "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else
            {
                std::cout<<"\t"<<'%'<<nowww<<"= ge "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }
        }
        int Calc() override
        {
            if(op=="<")
                return (re_exp->Calc())<(ad_exp->Calc());
            else if(op==">")
                return (re_exp->Calc())>(ad_exp->Calc());
            else if(op=="<=")
                return (re_exp->Calc())<=(ad_exp->Calc());
            else
                return (re_exp->Calc())>=(ad_exp->Calc());
        }
};

class EqExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> eq_exp;
        void Dump()override
        {
            eq_exp->Dump();
        }
        int Calc() override
        {
            return eq_exp->Calc();
        }
};

class MulEqExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> e_exp;
        std::string op;
        std::unique_ptr<BaseAST> re_exp;
        void Dump()override
        {
            e_exp->Dump();
            int now1=nowww-1;
            re_exp->Dump();
            int now2=nowww-1;
            if(op=="==")
            {
                std::cout<<"\t"<<'%'<<nowww<<"= eq "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }else
            {
                std::cout<<"\t"<<'%'<<nowww<<"= ne "<<'%'<<now1<<", %"<<now2<<std::endl;
                ++nowww;
            }
        }
        int Calc() override
        {
            if(op=="==")
                return (e_exp->Calc())==(re_exp->Calc());
            else
                return (e_exp->Calc())!=(re_exp->Calc());
        }
};

class LandExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> land_exp;
        void Dump()override
        {
            land_exp->Dump();
        }
        int Calc() override
        {
            return land_exp->Calc();
        }
};

class MulAndExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> and_exp;
        std::string op;
        std::unique_ptr<BaseAST> e_exp;
        void Dump()override
        {
            and_exp->Dump();
            int now1=nowww-1;
            int temp=nowww;
            std::cout<<"\t@result_"<<temp<<" = alloc i32"<<std::endl;
            std::cout<<"\t%"<<nowww<<"= ne 0, %"<<now1<<std::endl;
            std::cout<<"\tstore %"<<now1<<", @result_"<<temp<<std::endl;
            nowww++;
            
            if_cnt++;
            iff[if_cnt]=now_if;
            now_if=if_cnt;

            std::cout<<"\tbr %"<<now1<<", %then"<<now_if<<", %end"<<now_if<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%then"<<now_if<<":"<<std::endl;
            e_exp->Dump();
            int now2=nowww-1;
            std::cout<<"\t%"<<nowww<<"= ne 0, %"<<now2<<std::endl;
            nowww++;
            std::cout<<"\tstore "<<'%'<<nowww-1<<", @result_"<<temp<<std::endl;
            std::cout<<"\tjump %end"<<now_if<<std::endl;

            std::cout<<std::endl;
            std::cout<<"%end"<<now_if<<":"<<std::endl;
            std::cout<<"\t%"<<nowww<<"= load @result_"<<temp<<std::endl;
            nowww++;
            now_if=iff[now_if];
        }
        int Calc() override
        {
            return (and_exp->Calc())&&(e_exp->Calc());
        }
};

class LorExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> lor_exp;
        void Dump()override
        {
            lor_exp->Dump();
        }
        int Calc() override
        {
            return lor_exp->Calc();
        }
};

class MulLorExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> lo_exp;
        std::string op;
        std::unique_ptr<BaseAST> lan_exp;
        void Dump()override
        {
            lo_exp->Dump();
            int now1=nowww-1;
            int temp=nowww;
            std::cout<<"\t@result_"<<temp<<" = alloc i32"<<std::endl;
            std::cout<<"\t%"<<nowww<<"= ne 0, %"<<now1<<std::endl;
            std::cout<<"\tstore %"<<nowww<<", @result_"<<temp<<std::endl;
            nowww++;

            if_cnt++;
            iff[if_cnt]=now_if;
            now_if=if_cnt;

            std::cout<<"\tbr %"<<now1<<", %end"<<now_if<<", %then"<<now_if<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%then"<<now_if<<":"<<std::endl;
            lan_exp->Dump();
            int now2=nowww-1;
            std::cout<<"\t%"<<nowww<<"= ne 0, %"<<now2<<std::endl;
            nowww++;
            std::cout<<"\tstore "<<'%'<<nowww-1<<", @result_"<<temp<<std::endl;
            std::cout<<"\tjump %end"<<now_if<<std::endl;

            std::cout<<std::endl;
            std::cout<<"%end"<<now_if<<":"<<std::endl;
            std::cout<<"\t%"<<nowww<<"= load @result_"<<temp<<std::endl;
            nowww++;
            now_if=iff[now_if];
        }
        int Calc() override
        {
            return (lo_exp->Calc())||(lan_exp->Calc());
        }
};

class GloDecl:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> glo_decl;
        void Dump()override
        {
            glo_var=1;
            glo_decl->Dump();
            glo_var=0;
        }
        int Calc()override
        {
            return 33;
        }
};

class Decl:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> decl;
        void Dump()override
        {
            decl->Dump();
        }
        int Calc() override{ return 1; }
};

class ConstDecl:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> btype;
        std::unique_ptr<BaseAST> const_decl;
        void Dump()override
        {
            ///std::cout<<"int"<<std::endl;
            const_decl->Dump();
        }
        int Calc() override{ return 2; }
};

class MulConstDef:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> const_def;
        std::unique_ptr<BaseAST> mul_const_dcl;
        void Dump()override
        {
            const_def->Dump();
            mul_const_dcl->Dump();
        }
        int Calc() override{ return 3; }
};

class MulConstDecl:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> mul_const_def;
        void Dump()override
        {
           mul_const_def->Dump();
        }
        int Calc() override{ return 4; }
};

class ConstDef:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> const_init_val;
        int Calc()override
        {
            const_val[IDENT]=const_init_val->Calc();
            //std::cout<<const_val[IDENT]<<std::endl;
            return const_val[IDENT];
        }
        void Dump()override
        {
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
            var_type[IDENT]=0;
            Calc();
            //std::cout<<IDENT<<std::endl;
            //const_init_val->Dump();
        }
};

class ConstArrayDef:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> siz;
        std::unique_ptr<BaseAST> const_array_val;
        std::vector<int> size;
        void Dump()override
        {
            size=siz->Para();
            array_dim=size;
            filled_sum=0;
            if(!glo_var)
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"\t@"<<IDENT<<" = alloc ";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<std::endl;
                var_type[IDENT]=2;
            }else
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"global @"<<IDENT<<" = alloc ";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<", ";
                var_type[IDENT]=2;
            }
            if(glo_var)
            {
                
                std::vector<int> con_init_val=const_array_val->Para();
                auto it=con_init_val.begin();
                for(int i=0;i<size.size();i++)std::cout<<"{";
                std::vector<int> v;
                for(int i=size.size()-1;i>=0;i--)
                {
                    if(i==size.size()-1)v.push_back(size[i]);
                    else v.push_back(size[i]*v[v.size()-1]);
                }
                int p=0;
                while(it!=con_init_val.end())
                {
                    std::cout<<(*it);
                    p++;
                    int flag=0;
                    for(int i=0;i<v.size();i++)
                    {
                        if(p%v[i]==0)
                        {
                            flag++;
                            std::cout<<"}";
                        }else break;
                    }
                    it++;
                    if(it!=con_init_val.end())
                    {
                        std::cout<<", ";
                        if(flag)
                        {   
                            for(int i=0;i<flag;i++)std::cout<<"{";
                        }
                    }
                }
                std::cout<<std::endl;
            }else
            {
                std::vector<int> con_init_val=const_array_val->Para();
                int pos=0;
                std::vector<int> v;
                int rsize=1;
                for(auto it=size.begin();it!=size.end();it++)rsize*=(*it);
                for(auto it=size.begin();it!=size.end();it++)rsize/=(*it),v.push_back(rsize);
                std::cout<<"\t%"<<nowww<<"= add 0, 0"<<std::endl;
                int reg0=nowww;
                nowww++;
                for(auto it=con_init_val.begin();it!=con_init_val.end();it++,pos++)
                {
                    int temp=pos;
                    for(auto i=v.begin();i!=v.end();i++)
                    {
                        if(i==v.begin())
                            std::cout<<"\t%"<<nowww<<"= getelemptr @"<<IDENT<<", "<<temp/(*i)<<std::endl;
                        else 
                            std::cout<<"\t%"<<nowww<<"= getelemptr %"<<nowww-1<<", "<<temp/(*i)<<std::endl;
                        nowww++,temp%=(*i);
                    }
                    if((*it)!=0)
                        std::cout<<"\tstore %"<<(*it)<<", %"<<nowww-1<<std::endl;
                    else
                        std::cout<<"\tstore %"<<reg0<<", %"<<nowww-1<<std::endl;
                }
            }
            array_siz[IDENT]=size.size();
        }
        int Calc()override
        {
            return 35;
        }

};

class ArraySize:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> array_size;
        void Dump()override
        {
            array_size->Dump();
        }
        int Calc()override
        {
            return 46;
        }
        std::vector <int> Para()override
        {
            return array_size->Para();
        }
};

class MulArraySize:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_array_size;
        std::unique_ptr<BaseAST> mul_array_size;
        void Dump()override
        {
            sin_array_size->Dump();
            mul_array_size->Dump();
        }
        int Calc()override
        {
            return 47;
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret;
            std::vector<int> v1=sin_array_size->Para();
            std::vector<int> v2=mul_array_size->Para();
            for(auto it=v1.begin();it!=v1.end();it++)ret.push_back((*it));
            for(auto it=v2.begin();it!=v2.end();it++)ret.push_back((*it));
            return ret;
        }
};

class ConstArrayVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> const_array_val;
        void Dump()override
        {
            const_array_val->Dump();
        }
        int Calc()override
        {
            return 36;
        }
        std::vector<int> Para()override
        {
            brace_dep++;
            std::vector<int> ret;
            int temp=filled_sum;
            
            if(const_array_val)ret=const_array_val->Para();
            int siz=1;
            brace_dep--;
            int las=1,i;
            for(i=array_dim.size()-1;i>=0;i--)
            {
                las*=array_dim[i];
                if(temp%las)break;
            }
            for(int k=std::max(i+1,brace_dep);k<array_dim.size();k++)siz*=array_dim[k];
            while(ret.size()<siz)ret.push_back(0),filled_sum++;
            return ret;
        }
};

class ConstArrVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> const_arr_val;
        void Dump()override
        {

        }
        int Calc()override
        {
            return 47;
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret=const_arr_val->Para();
            return ret;
        }
};

class MulConArrVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_con_arr_val;
        std::unique_ptr<BaseAST> mul_con_arr_val;
        void Dump()override
        {
            
        }
        int Calc()override
        {
            return 37;
        }
        std::vector <int> Para()override
        {
            std::vector <int> ret;

            std::vector <int> v1=sin_con_arr_val->Para();
            std::vector <int> v2=mul_con_arr_val->Para();

            for(auto it=v1.begin();it!=v1.end();it++)ret.push_back((*it));
            for(auto it=v2.begin();it!=v2.end();it++)ret.push_back((*it));

            return ret;
        }
};

class VarArrayDef:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> var_array_def;
        void Dump()override
        {
            var_array_def->Dump();
        }
        int Calc()override
        {
            return 38;
        }
};

class SinNameVarArrDef:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> siz;
        std::vector<int> size;
        void Dump()override
        {
            size=siz->Para();
            if(!glo_var)
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"\t@"<<IDENT<<" = alloc";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<std::endl;
                var_type[IDENT]=2;
                
            }else
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"global @"<<IDENT<<" = alloc ";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<", ";
                for(int i=0;i<size.size();i++)std::cout<<"{";
                std::vector<int> v;
                for(int i=size.size()-1;i>=0;i--)
                {
                    if(i==size.size()-1)v.push_back(size[i]);
                    else v.push_back(size[i]*v[v.size()-1]);
                }
                int p=0;
                while(p<v[v.size()-1])
                {
                    std::cout<<0;
                    p++;
                    int flag=0;
                    for(int i=0;i<v.size();i++)
                    {
                        if(p%v[i]==0)
                        {
                            flag++;
                            std::cout<<"}";
                        }else break;
                    }
                    if(p!=v[v.size()-1])
                    {
                        std::cout<<", ";
                        if(flag)
                        {   
                            for(int i=0;i<flag;i++)std::cout<<"{";
                        }
                    }
                }
                std::cout<<std::endl;
                var_type[IDENT]=2;
            }
            array_siz[IDENT]=size.size();
        }
        int Calc()override
        {
            return 39;
        }
};

class MulNameVarArrDef:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> siz;
        std::unique_ptr<BaseAST> init_val;
        std::vector<int> size;
        void Dump()override
        {
            filled_sum=0;
            size=siz->Para();
            array_dim=size;
            if(!glo_var)
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"\t@"<<IDENT<<" = alloc ";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<std::endl;
                var_type[IDENT]=2;
            }else
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"global @"<<IDENT<<" = alloc ";
                for(auto it=size.begin();it!=size.end();it++)
                {
                    std::cout<<"[";
                }
                std::cout<<"i32";
                for(int it=size.size()-1;it>=0;it--)
                {
                    std::cout<<", "<<size[it]<<"]";
                }
                std::cout<<", ";
                var_type[IDENT]=2;
            }
            if(glo_var)
            {
                filled_sum=0;
                std::vector<int> con_init_val=init_val->Para();
                auto it=con_init_val.begin();
                for(int i=0;i<size.size();i++)std::cout<<"{";
                std::vector<int> v;
                for(int i=size.size()-1;i>=0;i--)
                {
                    if(i==size.size()-1)v.push_back(size[i]);
                    else v.push_back(size[i]*v[v.size()-1]);
                }
                int p=0;
                while(it!=con_init_val.end())
                {
                    std::cout<<(*it);
                    p++;
                    int flag=0;
                    for(int i=0;i<v.size();i++)
                    {
                        if(p%v[i]==0)
                        {
                            flag++;
                            std::cout<<"}";
                        }else break;
                    }
                    it++;
                    if(it!=con_init_val.end())
                    {
                        std::cout<<", ";
                        if(flag)
                        {   
                            for(int i=0;i<flag;i++)std::cout<<"{";
                        }
                    }
                }
                std::cout<<std::endl;
            }else
            {
                std::vector<int> con_init_val=init_val->Para();
                int pos=0;
                std::vector<int> v;
                int rsize=1;
                for(auto it=size.begin();it!=size.end();it++)rsize*=(*it);
                for(auto it=size.begin();it!=size.end();it++)rsize/=(*it),v.push_back(rsize);
                std::cout<<"\t%"<<nowww<<"= add 0, 0"<<std::endl;
                int reg0=nowww;
                nowww++;
                for(auto it=con_init_val.begin();it!=con_init_val.end();it++,pos++)
                {
                    int temp=pos;
                    for(auto i=v.begin();i!=v.end();i++)
                    {
                        if(i==v.begin())
                            std::cout<<"\t%"<<nowww<<"= getelemptr @"<<IDENT<<", "<<temp/(*i)<<std::endl;
                        else 
                            std::cout<<"\t%"<<nowww<<"= getelemptr %"<<nowww-1<<", "<<temp/(*i)<<std::endl;
                        nowww++,temp%=(*i);
                    }
                    if((*it)!=0)
                        std::cout<<"\tstore %"<<(*it)<<", %"<<nowww-1<<std::endl;
                    else
                        std::cout<<"\tstore %"<<reg0<<", %"<<nowww-1<<std::endl;
                }
            }
            array_siz[IDENT]=size.size();
        }
        int Calc()override
        {
            return 40;
        }
};

class VarArrInitVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> var_arr_init_val;
        void Dump()override
        {
            var_arr_init_val->Dump();
        }
        int Calc()override
        {
            return 41;
        }
        std::vector <int> Para()override
        {
            brace_dep++;
            std::vector<int> ret;
            int temp=filled_sum;
            if(var_arr_init_val)ret=var_arr_init_val->Para();
            int siz=1;
            brace_dep--;
            int las=1,i;
            for(i=array_dim.size()-1;i>=0;i--)
            {
                las*=array_dim[i];
                if(temp%las)break;
            }
            for(int k=std::max(i+1,brace_dep);k<array_dim.size();k++)siz*=array_dim[k];
            while(ret.size()<siz)ret.push_back(0),filled_sum++;
            return ret;
        }
};

class VarArrVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> var_arr_val;
        void Dump()override
        {

        }
        int Calc()override
        {
            return 48;
        }
        std::vector<int> Para()override
        {
            return var_arr_val->Para();
        }
};

class MulVarArrInitVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_var_arr_init_val;
        std::unique_ptr<BaseAST> mul_var_arr_init_val;
        void Dump()override
        {
            sin_var_arr_init_val->Dump();
            mul_var_arr_init_val->Dump();
        }
        int Calc()override
        {
            return 42;
        }
        std::vector<int> Para()override
        {
            std::vector <int> ret;

            std::vector <int> v1=sin_var_arr_init_val->Para();
            filled_sum+=v1.size();
            std::vector <int> v2=mul_var_arr_init_val->Para();
            filled_sum+=v2.size();
            for(auto it=v1.begin();it!=v1.end();it++)ret.push_back((*it));
            for(auto it=v2.begin();it!=v2.end();it++)ret.push_back((*it));

            return ret;
        }
};


class ConstInitVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> const_exp;
        void Dump()override
        {
            //const_exp->Dump();
        }
        int Calc() override
        {
            return const_exp->Calc();
        }
        std::vector<int> Para()override
        {
            std::vector <int> ret;
            if(glo_var)
            {
                ret.push_back(const_exp->Calc());
                filled_sum++;
                return ret;
            }else
            {
                const_exp->Dump();
                ret.push_back(nowww-1);
                filled_sum++;
                return ret;
            }
        }
};

class AllLval:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> all_lval;
        void Dump()override
        {
            all_lval->Dump();
        }
        int Calc()override
        {
            return all_lval->Calc();
        }
        std::vector<int> Para()override
        {
            return all_lval->Para();
        }
};

class AllLeval:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> all_leval;
        void Dump()override
        {
            all_leval->Dump();
        }
        int Calc()override
        {
            return 46;
        }
};

class ArrLval:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> pos_exp;
        void Dump()override
        {
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            std::vector<int> pos=pos_exp->Para();
            for(auto it=pos.begin();it!=pos.end();it++)
            {
                if(it==pos.begin())
                {
                    if(var_type[IDENT]==3)
                    {    
                        std::cout<<"\t%"<<nowww<<"= load @"<<IDENT<<std::endl;
                        nowww++;
                        std::cout<<"\t%"<<nowww<<"= getptr %"<<nowww-1<<", %"<<(*it)<<std::endl;
                    }else
                        std::cout<<"\t%"<<nowww<<"= getelemptr @"<<IDENT<<", %"<<(*it)<<std::endl;
                }
                else
                    std::cout<<"\t%"<<nowww<<"= getelemptr %"<<nowww-1<<", %"<<(*it)<<std::endl;
                nowww++;
            }
            if(!be_func_para||pos.size()==array_siz[IDENT])
            {
                std::cout<<"\t%"<<nowww<<"= load %"<<nowww-1<<std::endl;
                nowww++;
            }else
            {
                //std::cout<<be_func_para<<" "<<pos.size()<<" "<<IDENT<<std::endl;
                std::cout<<"\t%"<<nowww<<"= getelemptr %"<<nowww-1<<", 0"<<std::endl;
                nowww++;
            }
        }
        int Calc()override
        {
            return 43;
        }
};

class ArrLeval:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> pos_exp;
        void Dump()override
        {
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            int now=nowww-1;
            std::vector<int> pos=pos_exp->Para();
            for(auto it=pos.begin();it!=pos.end();it++)
            {
                if(it==pos.begin())
                {
                    if(var_type[IDENT]==3)
                    {    
                        std::cout<<"\t%"<<nowww<<"= load @"<<IDENT<<std::endl;
                        nowww++;
                        std::cout<<"\t%"<<nowww<<"= getptr %"<<nowww-1<<", %"<<(*it)<<std::endl;
                    }else
                        std::cout<<"\t%"<<nowww<<"= getelemptr @"<<IDENT<<", %"<<(*it)<<std::endl;
                }else
                    std::cout<<"\t%"<<nowww<<"= getelemptr %"<<nowww-1<<", %"<<(*it)<<std::endl;
                nowww++;
            }
            std::cout<<"\tstore %"<<now<<", %"<<nowww-1<<std::endl;
            nowww++;
        }
        int Calc()override
        {
            return 44;
        }
};

class ArrPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> arr_para;
        void Dump()override
        {
            arr_para->Dump();
        }
        int Calc()override
        {
            return 49;
        }
        std::vector<int> Para()override
        {
            return arr_para->Para();
        }
};

class MulArrPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_arr_para;
        std::unique_ptr<BaseAST> mul_arr_para;
        void Dump()override
        {
            sin_arr_para->Dump();
            mul_arr_para->Dump();
        }
        int Calc()override
        {
            return 50;
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret;
            std::vector<int> v1=sin_arr_para->Para();
            std::vector<int> v2=mul_arr_para->Para();

            for(auto it=v1.begin();it!=v1.end();it++)ret.push_back((*it));
            for(auto it=v2.begin();it!=v2.end();it++)ret.push_back((*it));

            return ret;
        }
};

class Lval:public BaseAST
{
    public:
        std::string IDENT;
        void Dump()override
        {
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            if(var_type[IDENT]==0)
            {
                std::cout<<"\t%"<<nowww<<"= add "<<"0 ,"<<const_val[IDENT]<<std::endl;
                nowww++;
            }else if(var_type[IDENT]==2)
            {
                std::cout<<"\t%"<<nowww<<"= getelemptr @"<<IDENT<<", 0"<<std::endl;
                nowww++;
            }
            else
            {
                std::cout<<"\t%"<<nowww<<"= load "<<"@"<<IDENT<<std::endl;
                nowww++;
            }
        }
        int Calc() override
        {
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            return const_val[IDENT];
        }
        std::vector<int> Para()override
        {
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            std::vector<int> ret;
            ret.push_back(const_val[IDENT]);
            return ret;
        }
};

class LeVal:public BaseAST
{
    public:
        std::string IDENT;
        void Dump()override
        {
            //assert(var_type[IDENT]!=0);
            int tempdep=nowdep;
            while(var_type.find("COMPILER__"+IDENT+"_"+std::to_string(tempdep))==var_type.end())tempdep=f[tempdep];
            IDENT="COMPILER__"+IDENT+"_"+std::to_string(tempdep);
            std::cout<<"\tstore %"<<nowww-1<<", @"<<IDENT<<std::endl;

        }
        int Calc()override
        {
            return 0;
        }
};

class ConstExp:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> const_exp;
        void Dump()override
        {
            const_exp->Dump();
        }
        int Calc() override
        {
            return const_exp->Calc();
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret;
            ret.push_back(const_exp->Calc());
            return ret;
        }
};

class VarDecl:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> var_decl;
        void Dump()override
        {
            var_decl->Dump();
        }
        int Calc()override
        {
            return 12;
        }
};

class VarDef:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> var_def;
        void Dump()override
        {
            var_def->Dump();
        }
        int Calc()override
        {
            return 13;
        }
};

class SinVarName:public BaseAST
{
    public:
        std::string IDENT;
        void Dump()override
        {
            if(!glo_var)
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"\t@"<<IDENT<<" = alloc i32"<<std::endl;
                var_type[IDENT]=1;
                const_val[IDENT]=0;
            }else
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"global @"<<IDENT<<" = alloc i32, 0"<<std::endl;
                var_type[IDENT]=1;
                const_val[IDENT]=0;
            }
        }
        int Calc()override
        {
            return 0;
        }
};

class MulVarName:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> init_val;
        void Dump()override
        {
            if(!glo_var)
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                std::cout<<"\t@"<<IDENT<<" = alloc i32"<<std::endl;
           
                var_type[IDENT]=1;
                init_val->Dump();
                std::cout<<"\tstore %"<<nowww-1<<", @"<<IDENT<<std::endl; 
            }else
            {
                IDENT="COMPILER__"+IDENT+"_"+std::to_string(nowdep);
                var_type[IDENT]=1;
                const_val[IDENT]=init_val->Calc();
                std::cout<<"global @"<<IDENT<<" = alloc i32, "<<const_val[IDENT]<<std::endl;
            }
        }
        int Calc()override
        {
            return const_val[IDENT];
        }
};

class InitVal:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> init_exp;
        void Dump()override
        {
            init_exp->Dump();
        }
        int Calc()override
        {
            return init_exp->Calc();
        }
        std::vector <int> Para()override
        {
            std::vector <int> ret;
            if(glo_var)
            {
                ret.push_back(init_exp->Calc());
                filled_sum++;
                return ret;
            }else
            {
                init_exp->Dump();
                ret.push_back(nowww-1);
                filled_sum++;
                return ret;
            }
        }
};

class SinVarDef:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_var_def;
        void Dump()override
        {
            sin_var_def->Dump();
        }
        int Calc()override
        {
            return 17;
        }
};

class MulVarDef:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_var;
        std::unique_ptr<BaseAST> mul_var;
        void Dump()override
        {
            sin_var->Dump();
            mul_var->Dump();
        }
        int Calc()override
        {
            return 18;
        }
};

class IfStmt:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> if_stm;
        void Dump()override
        {
            if_stm->Dump();
        }
        int Calc()override
        {
            return 19;
        }
};

class SinIfStmt:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> if_exp;
        std::unique_ptr<BaseAST> if_stmt;
        void Dump()override
        {
            if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
            if_cnt++;
            iff[if_cnt]=now_if;
            now_if=if_cnt;
            
            if_exp->Dump();
            
            
            std::cout<<"\tbr %"<<nowww-1<<", %then"<<now_if<<", %end"<<now_if<<std::endl;
            std::cout<<std::endl;
            
            
            std::cout<<"%then"<<now_if<<":"<<std::endl;
            
            dep++;
            f[dep]=nowdep;
            nowdep=dep;

            bl_dep++;
            blf[bl_dep]=nowbl;
            nowbl=bl_dep;

            
            if_stmt->Dump();
            
            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"\tjump %end"<<now_if<<std::endl;

            
            std::cout<<std::endl;
            std::cout<<"%end"<<now_if<<":"<<std::endl;
            nowdep=f[nowdep],nowbl=blf[nowbl];
            now_if=iff[now_if];
        }
        int Calc()override
        {
            return 20;
        }
};

class MulIfStmt:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> if_exp;
        std::unique_ptr<BaseAST> if_stmt;
        std::unique_ptr<BaseAST> el_stmt;
        void Dump()override
        {
            if(be_end_dep[nowdep]||be_end_bl[nowbl])return;
            if_cnt++;
            iff[if_cnt]=now_if;
            now_if=if_cnt;
            

            if_exp->Dump();
            std::cout<<"\tbr %"<<nowww-1<<", %then"<<now_if<<", %else"<<now_if<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%then"<<now_if<<":"<<std::endl;
            
            dep++;
            f[dep]=nowdep;
            nowdep=dep;

            bl_dep++;
            blf[bl_dep]=nowbl;
            nowbl=bl_dep;
            
            if_stmt->Dump();
            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"\tjump %end"<<now_if<<std::endl;

            nowdep=f[nowdep]; 
            dep++;
            f[dep]=nowdep;
            nowdep=dep;

            nowbl=blf[nowbl];
            bl_dep++;
            blf[bl_dep]=nowbl;
            nowbl=bl_dep;

            std::cout<<std::endl;
            std::cout<<"%else"<<now_if<<":"<<std::endl;

            el_stmt->Dump();

            
            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"\tjump %end"<<now_if<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%end"<<now_if<<":"<<std::endl;
            now_if=iff[now_if];
            nowdep=f[nowdep];
            nowbl=blf[nowbl];
        }
        int Calc()override
        {
            return 21;
        }
};

class WhileStmt:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> while_exp;
        std::unique_ptr<BaseAST> while_stmt;
        void Dump()override
        {
            wh_cnt++;
            whf[wh_cnt]=now_wh;
            now_wh=wh_cnt;

            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"jump %whilecheck"<<now_wh<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%whilecheck"<<now_wh<<":"<<std::endl;

            while_exp->Dump();
            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"\tbr %"<<nowww-1<<", %whilethen"<<now_wh<<", %endwhile"<<now_wh<<std::endl;
            std::cout<<std::endl;
            std::cout<<"%whilethen"<<now_wh<<":"<<std::endl;

            bl_dep++;
            blf[bl_dep]=nowbl;
            nowbl=bl_dep;

            dep++;
            f[dep]=nowdep;
            nowdep=dep;

            while_stmt->Dump();
            if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])std::cout<<"\tjump %whilecheck"<<now_wh<<std::endl;

            std::cout<<std::endl;
            std::cout<<"%endwhile"<<now_wh<<":"<<std::endl;
            nowbl=blf[nowbl];
            now_wh=whf[now_wh];
            nowdep=f[nowdep];
        }
        int Calc()override
        {
            return 22;
        }
};

class ConWhile:public BaseAST
{
    public:
        std::string str;
        void Dump()override
        {
            if(str=="break")
            {
                if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])
                {
                    std::cout<<"\tjump %endwhile"<<now_wh<<std::endl;
                    be_end_dep[nowdep]=be_end_bl[nowbl]=1; 
                }
               
            }else
            {
                if(!be_end_dep[nowdep]&&!be_end_bl[nowbl])
                {
                    std::cout<<"\tjump %whilecheck"<<now_wh<<std::endl;
                    be_end_dep[nowdep]=be_end_bl[nowbl]=1;
                }
                
            }
        }
        int Calc()override
        {
            return 23;
        }
};

class FuncExp:public BaseAST
{
    public:
        std::string IDENT;
        std::unique_ptr<BaseAST> call_para;
        int typ;
        void Dump()override
        {
            if(typ)
            {
                be_func_para=1;
                std::vector<int> paras=call_para->Para();
                if(IDENT=="getint"||IDENT=="getch"||IDENT=="getarray"||func_ret[IDENT])
                {
                    std::cout<<"    %"<<nowww<<"=call @"<<IDENT<<"(";
                    nowww++;
                }else std::cout<<"    call @"<<IDENT<<"(";
                for(auto it=paras.begin();it!=paras.end();it++)
                {
                    if(it!=paras.begin())std::cout<<',';
                    std::cout<<"%"<<*it;
                }
                std::cout<<')'<<std::endl;
                be_func_para=0;
            }else 
            {
                if(IDENT=="getint"||IDENT=="getch"||IDENT=="getarray"||func_ret[IDENT])
                {
                    std::cout<<"    %"<<nowww<<"=call @"<<IDENT<<"()"<<std::endl;
                    nowww++;
                }else std::cout<<"    call @"<<IDENT<<"()"<<std::endl;
            }
        }
        int Calc()override
        {
            return 27;
        }
};

class CallPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> call_para;
        void Dump()override
        {
            call_para->Dump();
        } 
        int Calc()override
        {
            return 28;
        }
        std::vector<int> Para()override
        {
            return call_para->Para();
        }
};

class SinCallPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST>para_exp;
        void Dump()override
        {
            para_exp->Dump();
        }
        int Calc()override
        {
            return 29;
        }
        std::vector<int> Para()override
        {
            std::vector<int> ret;
            Dump();
            ret.push_back(nowww-1);
            return ret;
        }
};

class MulCallPara:public BaseAST
{
    public:
        std::unique_ptr<BaseAST> sin_call_para;
        std::unique_ptr<BaseAST> mul_call_para;
        void Dump()override
        {
            sin_call_para->Dump();
            mul_call_para->Dump();
        }
        int Calc()override
        {
            return 30;
        }
        std::vector<int> Para()override
        {
            std::vector <int> ret;
            std::vector <int> re1=sin_call_para->Para();
            std::vector <int> re2=mul_call_para->Para();
            for(auto it=re1.begin();it!=re1.end();it++)ret.push_back(*it);
            for(auto it=re2.begin();it!=re2.end();it++)ret.push_back(*it);
            return ret;
        }
};