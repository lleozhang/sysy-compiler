#include <array>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
#include "koopa.h"
#include <stdio.h>
#include <unordered_map>
using namespace std;
typedef unsigned long long ull;
// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
unordered_map <ull,int> M;
unordered_map <ull,vector <int> >array_size;
ull now_array;
static int deep;
void solve_load(koopa_raw_value_t value,int &st)
{
    if(value->kind.data.load.src->kind.tag==KOOPA_RVT_GLOBAL_ALLOC)
    {
        cout<<"   la t0, "<<value->kind.data.load.src->name+1<<endl;
        cout<<"   lw t0, 0(t0)"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, (t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
        
    }else if(value->kind.data.load.src->kind.tag==KOOPA_RVT_GET_ELEM_PTR)
    {
        cout<<"   li t4, "<<M[(ull)value->kind.data.load.src]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        cout<<"   lw t0, 0(t0)"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, (t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(value->kind.data.load.src->kind.tag==KOOPA_RVT_GET_PTR)
    {
        cout<<"   li t4, "<<M[(ull)value->kind.data.load.src]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        cout<<"   lw t0, 0(t0)"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, (t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else
    {
        array_size[(ull)value]=array_size[(ull)(value->kind.data.load.src)];
        //cout<<value->kind.data.load.src->kind.tag<<" "<<M[(ull)(value->kind.data.load.src)]<<endl;
        M[(ull)value]=M[(ull)(value->kind.data.load.src)];
    }
}
void slice_value(koopa_raw_value_t l,koopa_raw_value_t r,int &st,int &lreg,int &rreg)
{
    if(l->kind.tag==KOOPA_RVT_LOAD)solve_load(l,st);
    if(r->kind.tag==KOOPA_RVT_LOAD)solve_load(r,st);
    if(l->kind.tag==KOOPA_RVT_INTEGER)
    {
      if(l->kind.data.integer.value==0)
      {
          lreg=-1;
      }else
      {  
          cout<<"   li t0"<<","<<l->kind.data.integer.value<<endl;
          lreg=0;
      }
    }else 
    {
        cout<<"   li t4, "<<M[(ull)l]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        lreg=0;
    }
    if(r->kind.tag==KOOPA_RVT_INTEGER)
    {
        if(r->kind.data.integer.value==0)rreg=-1;
        else
        {
            cout<<"   li t1"<<","<<l->kind.data.integer.value<<endl;
            rreg=1;
        }
    }else
    {
        cout<<"   li t4, "<<M[(ull)r]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t1, (t4)"<<endl;
        rreg=1;
    }
}

void print(int lreg,int rreg)
{
    if(lreg==-1)
      cout<<"x0";
    else
      cout<<"t"<<lreg;
      
    if(rreg==-1) 
      cout<<", x0"<<endl;
    else
      cout<<", t"<<rreg<<endl;
}

void solve_binary(koopa_raw_value_t value,int &st)
{
    koopa_raw_binary_t exp=value->kind.data.binary;
    koopa_raw_value_t l=exp.lhs;
    koopa_raw_value_t r=exp.rhs;
    int lreg,rreg;
    if(exp.op==6)//加法
    {
        bool flag=0;
        int lreg,rreg;
        if(l->kind.tag==KOOPA_RVT_INTEGER)
        {
            if(l->kind.data.integer.value==0)
            {
                cout<<"   li t0"<<", "<<r->kind.data.integer.value<<endl;
                cout<<"   li t5, "<<st<<endl;
                cout<<"   add t5, t5, sp"<<endl;
                cout<<"   sw t0, (t5)"<<endl;
                M[(ull)value]=st;
                st+=4;
                return;
            }
        }
        slice_value(l,r,st,lreg,rreg);
        cout<<"   add t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==7)//减法
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   sub t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==1)//比较恒等
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   xor t2"<<", ";
        print(lreg,rreg);

        cout<<"   seqz t2, t2"<<endl;
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==8)//乘法
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   mul t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==9)//除法
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   div t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==10)//取模
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   rem t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==0)//比较不等于
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   xor t2"<<", ";
        print(lreg,rreg);

        cout<<"   snez t2, t2"<<endl;
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==2)//比较大于
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   sgt t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==3)//比较小于
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   slt t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==4)//比较大于等于‘
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   sgt t2"<<", ";
        print(lreg,rreg);
        //slice_value(l, r, lreg, rreg, noww);

        cout<<"   xor t3"<<", ";
        print(lreg,rreg);

        cout<<"   seqz t3, t3"<<endl;

        cout<<"   or t4, t2, t3"<<endl;
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t4, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==5)//比较小于等于
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   slt t2"<<", ";
        print(lreg,rreg);
        //slice_value(l, r, lreg, rreg, noww);

        cout<<"   xor t3"<<", ";
        print(lreg,rreg);

        cout<<"   seqz t3, t3"<<endl;

        cout<<"   or t4, t2, t3"<<endl;
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t4, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==11)//与
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   and t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(exp.op==12)//或
    {
        slice_value(l,r,st,lreg,rreg);
        cout<<"   or t2"<<", ";
        print(lreg,rreg);
        cout<<"   li t5, "<<st<<endl;
        cout<<"   add t5, t5, sp"<<endl;
        cout<<"   sw t2, (t5)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }
          
}

void solve_return(koopa_raw_value_t value,int &st)
{
    koopa_raw_value_t ret_value = value->kind.data.ret.value;
    if(!ret_value)
    {
        cout<<"   li a0, 0"<<endl;
        cout<<"   li t4, 65532"<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw ra, (t4)"<<endl;
        cout<<"   li t4, 65536"<<endl;
        cout<<"   add sp, sp, t4"<<endl;
        cout<<"   ret"<<endl;
        return;
    }else if(ret_value->kind.tag == KOOPA_RVT_INTEGER)
    {
        int32_t int_val = ret_value->kind.data.integer.value;
        cout<<"   li "<<"a0 , "<<int_val<<endl;
        cout<<"   li t4, 65532"<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw ra, (t4)"<<endl;
        cout<<"   li t4, 65536"<<endl;
        cout<<"   add sp, sp, t4"<<endl;
        cout<<"   ret"<<endl;
    }else 
    {
        if(ret_value->kind.tag==KOOPA_RVT_LOAD)solve_load(ret_value,st);
        cout<<"   li t4, "<<M[(ull)ret_value]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw "<<"a0 , (t4)"<<endl;
        cout<<"   li t4, 65532"<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw ra, (t4)"<<endl;
        cout<<"   li t4, 65536"<<endl;
        cout<<"   add sp, sp, t4"<<endl;
        cout<<"   ret"<<endl;
    }
}

void solve_store(koopa_raw_value_t value,int &st)
{
    koopa_raw_store_t sto=value->kind.data.store;
    koopa_raw_value_t sto_value=sto.value;
    koopa_raw_value_t sto_dest=sto.dest;
    if(sto_value->kind.tag==KOOPA_RVT_INTEGER)
    {
        cout<<"   li t0, "<<sto_value->kind.data.integer.value<<endl;
    
    }else if(sto_value->kind.tag==KOOPA_RVT_FUNC_ARG_REF)
    {
        koopa_raw_func_arg_ref_t arg=sto_value->kind.data.func_arg_ref;
        if(arg.index<8)
            cout<<"   mv t0, a"<<arg.index<<endl;
        else
        {
            cout<<"   li t4, "<<65536+(arg.index-8)*4<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t0, (t4)"<<endl;
        }
    }else
    {
        if(sto_value->kind.tag==KOOPA_RVT_LOAD)solve_load(sto_value,st);
        cout<<"   li t4, "<<M[(ull)sto_value]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
    }
    if(sto_dest->kind.tag==KOOPA_RVT_GLOBAL_ALLOC)
    {
        cout<<"   la t1, "<<sto_dest->name+1<<endl;
        cout<<"   sw t0, 0(t1)"<<endl;
    }else if(sto_dest->kind.tag==KOOPA_RVT_GET_ELEM_PTR)
    {
        cout<<"   li t4, "<<M[(ull)sto_dest]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t1, (t4)"<<endl;
        cout<<"   sw t0, 0(t1)"<<endl;
    }else if(sto_dest->kind.tag==KOOPA_RVT_GET_PTR)
    {
        cout<<"   li t4, "<<M[(ull)sto_dest]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t1, (t4)"<<endl;
        cout<<"   sw t0, 0(t1)"<<endl;
    }else
    {
        if(M.find((ull)sto_dest)==M.end())
        {
            M[(ull)sto_dest]=st;
            st+=4;
        }
        cout<<"   li t4, "<<M[(ull)sto_dest]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, (t4)"<<endl;
    }
}

void solve_branch(koopa_raw_value_t value,int &st)
{
    koopa_raw_branch_t bran=value->kind.data.branch;
    if(bran.cond->kind.tag==KOOPA_RVT_LOAD)solve_load(bran.cond,st);
    cout<<"   li t4, "<<M[(ull)bran.cond]<<endl;
    cout<<"   add t4, t4, sp"<<endl;
    cout<<"   lw t3, (t4)"<<endl;
    cout<<"   beqz t3, "<<bran.false_bb->name+1<<endl;
    cout<<"   bnez t3, "<<bran.true_bb->name+1<<endl;
    cout<<endl;
}

void solve_jump(koopa_raw_value_t value,int &st)
{
    koopa_raw_jump_t jum=value->kind.data.jump;
    cout<<"   j "<<jum.target->name+1<<endl;
    cout<<endl;
}

void solve_call(koopa_raw_value_t value,int &st)
{
    koopa_raw_function_t func=value->kind.data.call.callee;
    koopa_raw_slice_t args=value->kind.data.call.args;
    int nowst=0;
    for(int i=1;i<=args.len;i++)
    {
        if(M.find((ull)(args.buffer[i-1]))!=M.end())
        {
            if(i<=8)
            {
                cout<<"   li t4, "<<M[(ull)(args.buffer[i-1])]<<endl;
                cout<<"   add t4, t4, sp"<<endl;
                cout<<"   lw a"<<i-1<<", (t4)"<<endl;
            }else
            {
                cout<<"   li t4, "<<M[(ull)args.buffer[i-1]]<<endl;
                cout<<"   add t4, t4, sp"<<endl;
                cout<<"   lw t0, (t4)"<<endl;
                cout<<"   li t4, "<<nowst<<endl;
                cout<<"   add t4, t4, sp"<<endl;
                cout<<"   sw t0, (t4)"<<endl;
                nowst+=4;
            }
        } 
       
    }
    cout<<"   call "<<func->name+1<<endl;
    M[(ull)value]=st;
    st+=4;
    cout<<"   li t4, "<<M[(ull)value]<<endl;
    cout<<"   add t4, t4, sp"<<endl;
    cout<<"   sw a0, (t4)"<<endl;
}

void solve_get_element_ptr(koopa_raw_value_t value,int &st)
{
    koopa_raw_value_t src=value->kind.data.get_elem_ptr.src;
    if(src->kind.tag==KOOPA_RVT_ALLOC)
    {
        now_array=(ull)src;
        deep=1;
        cout<<"   li t4, "<<M[(ull)src]<<endl;
        cout<<"   add t0, sp, t4"<<endl;
        if(value->kind.data.get_elem_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_elem_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_elem_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(src->kind.tag==KOOPA_RVT_GLOBAL_ALLOC)
    {
        now_array=(ull)src;
        deep=1;
        cout<<"   la t0, "<<value->kind.data.get_elem_ptr.src->name+1<<endl;
        if(value->kind.data.get_elem_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_elem_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_elem_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        //cout<<"??"<<array_size[now_array].size()<<endl;
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(src->kind.tag==KOOPA_RVT_GET_ELEM_PTR||src->kind.tag==KOOPA_RVT_GET_PTR)
    {
        cout<<"   li t4, "<<M[(ull)src]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        if(value->kind.data.get_elem_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_elem_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_elem_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        deep++;
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }
}
void solve_alloc(koopa_raw_value_t value,int &st)
{
    if(value->ty->data.pointer.base->tag==KOOPA_RTT_INT32)M[(ull)value]=st,st+=4;
    else if(value->ty->data.pointer.base->tag==KOOPA_RTT_ARRAY)
    {
        koopa_raw_type_kind_t* base=(koopa_raw_type_kind_t*)value->ty->data.pointer.base;
        array_size[(ull)value].push_back(base->data.array.len);
        int siz=base->data.array.len*4;
        while(base->data.array.base->tag!=KOOPA_RTT_INT32)
        {
            base=(koopa_raw_type_kind_t*)base->data.array.base;
            array_size[(ull)value].push_back(base->data.array.len);
            siz*=base->data.array.len;
        }
        M[(ull)value]=st;
        st+=siz;
    }else if(value->ty->data.pointer.base->tag==KOOPA_RTT_POINTER)
    {
        koopa_raw_type_kind_t* base=(koopa_raw_type_kind_t*)value->ty->data.pointer.base;
        array_size[(ull)value].push_back(1);
        while(base->data.array.base->tag!=KOOPA_RTT_INT32)
        {
            base=(koopa_raw_type_kind_t*)base->data.array.base;
            array_size[(ull)value].push_back(base->data.array.len);
        }
        M[(ull)value]=st;
        st+=4;
    }
}

void solve_global_array(koopa_raw_value_t value,koopa_raw_value_t ori)
{
    if(value->kind.tag==KOOPA_RVT_INTEGER)
    {
        cout<<"   .word "<<value->kind.data.integer.value<<endl;
        return;
    }else
    {
        koopa_raw_slice_t elems=value->kind.data.aggregate.elems;
        array_size[(ull)ori].push_back(elems.len);
        for(int i=0;i<elems.len;i++)
        {
            koopa_raw_value_t val=(koopa_raw_value_t)elems.buffer[i];
            solve_global_array(val,ori);
        }
        return;
    }
}

void solve_get_ptr(koopa_raw_value_t value,int &st)
{
    koopa_raw_value_t src=value->kind.data.get_ptr.src;
    if(src->kind.tag==KOOPA_RVT_ALLOC||src->kind.tag==KOOPA_RVT_LOAD)
    {
        now_array=(ull)src;
        deep=1;
        cout<<"   li t4, "<<M[(ull)src]<<endl;
        cout<<"   add t4, sp, t4"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        if(value->kind.data.get_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_ptr.index->kind.data.integer.value<<endl;
        else
        {    
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(src->kind.tag==KOOPA_RVT_GLOBAL_ALLOC)
    {
        now_array=(ull)src;
        deep=1;
        cout<<"   la t0, "<<value->kind.data.get_ptr.src->name+1<<endl;
        if(value->kind.data.get_elem_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        //cout<<"??"<<array_size[now_array].size()<<endl;
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else if(src->kind.tag==KOOPA_RVT_GET_ELEM_PTR)
    {
        
        cout<<"   li t4, "<<M[(ull)src]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        if(value->kind.data.get_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        deep++;
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }else
    {
        now_array=(ull)src;
        cout<<"   li t4, "<<M[(ull)src]<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   lw t0, (t4)"<<endl;
        if(value->kind.data.get_ptr.index->kind.tag==KOOPA_RVT_INTEGER)
            cout<<"   li t1, "<<value->kind.data.get_ptr.index->kind.data.integer.value<<endl;
        else
        {
            cout<<"   li t4, "<<M[(ull)value->kind.data.get_ptr.index]<<endl;
            cout<<"   add t4, t4, sp"<<endl;
            cout<<"   lw t1, "<<"(t4)"<<endl;
        }
        deep++;
        int p=4;
        for(int i=deep;i<array_size[now_array].size();i++)p*=array_size[now_array][i];
        cout<<"   li t2, "<<p<<endl;
        cout<<"   mul t1, t1, t2"<<endl;
        cout<<"   add t0, t0, t1"<<endl;
        cout<<"   li t4, "<<st<<endl;
        cout<<"   add t4, t4, sp"<<endl;
        cout<<"   sw t0, "<<"(t4)"<<endl;
        M[(ull)value]=st;
        st+=4;
    }
}

void parse_string(const char* str)
{
  // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);
    
    if(raw.values.len)
    {
        cout<<"   .data"<<endl;
        for(size_t i=0;i<raw.values.len;++i)
        {
            koopa_raw_value_t data=(koopa_raw_value_t)raw.values.buffer[i];
            cout<<"   .globl "<<data->name+1<<endl;
            cout<<data->name+1<<":"<<endl;
            if(data->kind.data.global_alloc.init->kind.tag==KOOPA_RVT_INTEGER)
            {
                cout<<"   .word "<<data->kind.data.global_alloc.init->kind.data.integer.value<<endl;
                cout<<endl;
            }else if(data->kind.data.global_alloc.init->kind.tag==KOOPA_RVT_AGGREGATE)
            {
                koopa_raw_slice_t elems=data->kind.data.global_alloc.init->kind.data.aggregate.elems;
                //array_size[(ull)data].push_back(elems.len);
                for(int i=0;i<elems.len;i++)
                {
                    koopa_raw_value_t val=(koopa_raw_value_t)elems.buffer[i];
                    solve_global_array(val,data);
                }
                cout<<endl;
            }
        }
    }
    cout<<endl;
   
    cout<<"   .text"<<endl;
    for (size_t i = 0; i < raw.funcs.len; ++i) 
    {
    // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
    // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        // 获取当前函数
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
        if(func->bbs.len==0)continue;
        cout<<endl<<"   .globl "<<func->name+1<<endl;
        cout<<func->name+1<<":"<<endl;
        cout<<"   li t0, -65536"<<endl;
        cout<<"   add sp, sp, t0"<<endl;
        cout<<"   li t0, 65532"<<endl;
        cout<<"   add t0, sp, t0"<<endl;
        cout<<"   sw ra, (t0)"<<endl;
        int st=2048;
        for (size_t j = 0; j < func->bbs.len; ++j)
        {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[j];
            if(bb->name)cout<<endl<<bb->name+1<<":"<<endl;
            for (size_t k = 0; k < bb->insts.len; ++k)
            {
                koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[k];
                if(value->kind.tag == KOOPA_RVT_RETURN)
                {
                    solve_return(value,st);
                }else if(value->kind.tag==KOOPA_RVT_BINARY)
                {
                    solve_binary(value,st);
                }else if(value->kind.tag==KOOPA_RVT_LOAD)
                {
                    solve_load(value,st);
                }else if(value->kind.tag==KOOPA_RVT_STORE)
                {
                    solve_store(value,st);
                }else if(value->kind.tag==KOOPA_RVT_BRANCH)
                {
                    solve_branch(value,st);
                }else if(value->kind.tag==KOOPA_RVT_JUMP)
                {
                    solve_jump(value,st);
                }else if(value->kind.tag==KOOPA_RVT_CALL)
                {
                    solve_call(value,st);
                }else if(value->kind.tag==KOOPA_RVT_GET_ELEM_PTR)
                {
                    solve_get_element_ptr(value,st);
                }else if(value->kind.tag==KOOPA_RVT_ALLOC)
                {
                    solve_alloc(value,st);
                }else if(value->kind.tag==KOOPA_RVT_GET_PTR)
                {
                    solve_get_ptr(value,st);
                }
            }
        }
    }
    koopa_delete_raw_program_builder(builder);
}

void Koopa_Dump()
{
    cout<<"decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()"<<endl;
}
int main(int argc, const char *argv[])
{
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
   yyin = fopen(input, "r");
    assert(yyin);

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);
    freopen("whatever.txt","w",stdout);
    //freopen(output,"w",stdout);
    // 输出解析得到的 AST, 其实就是个字符串
    Koopa_Dump();
    cout<<endl;
    ast->Dump();

    FILE* ff=fopen("whatever.txt","r");
    char *buf=(char *)malloc(10000000);
    fread(buf, 1,10000000, ff);
    //freopen("whatever.txt","w",stdout);
    freopen(output,"w",stdout);
    //printf("%s\n",buf);
    parse_string(buf);
    return 0;
}