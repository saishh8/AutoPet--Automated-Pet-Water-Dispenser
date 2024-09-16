#from django.shortcuts import render
#from django.http import HttpResponse
# Create your views here.

#def home(request):
#    return render(request,'newpet2.html')

from json import dumps
from math import fabs
from unicodedata import name
from django.shortcuts import redirect, render
import mysql.connector as sql
from pymysql import NULL
from django.http import HttpResponseRedirect
from django.contrib import messages
# Create your views here.
uname=''
email=''
password=''



def home(request):
    global uname,email,password,uid
    

    if request.method =="POST":
        if 'reg' in request.POST:
            cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
            cursor=cnx.cursor()
            d=request.POST
            for key,value in d.items():
                if key=="uname":
                    uname=value
                if key=="email":
                    email=value
                if key=="password":
                    password=value
            
            #query="insert into newsignedup values ('{}','{}','{}')".format(uname,password,email)
            query="insert into newsignedup (uname,pass,email) values ('{}','{}','{}')".format(uname,password,email)
            cursor.execute(query)
            cnx.commit()

            query2="select * from newsignedup where uname='{}' and pass='{}'".format(uname,password)
            cursor.execute(query2)
            t=list(cursor.fetchall())
            print(t)
            print(t[0])
            uid=t[0][0]
            return redirect('/newdashboard')
        
        if 'log' in request.POST:
            cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
            cursor=cnx.cursor()
            d=request.POST
            for key,value in d.items():
                if key=="logemail":
                    uname=value
                if key=="logpass":
                    password=value
            print('HE'+uname)
            query2="select * from newsignedup where uname='{}' and pass='{}'".format(uname,password)
            cursor.execute(query2)
            t=list(cursor.fetchall())
            print(t)
            #print(t[0][0])
            
            if t==[]:
                #return render(request,'mopage.html')
                return render(request,'newpet2.html',context={
        'sent': True})
                
            else:
                uid=t[0][0]
                #return render(request,'newpetprofile.html')
                return redirect('/dashboard')


    return render(request,'newpet2.html')


def dashboard(request):
    global owner_name,email,owner_no,pet_name,age,color,owner_id
    print('insertion success')
    print(request.POST)
    if request.method =="POST":
        print('insertion success post')
        if 'count1' in request.POST:
            cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
            cursor=cnx.cursor()
            a=request.POST
            print(a)
            for key,value in a.items():
                if key=="owner_name":
                    owner_name=value
                if key=="email":
                    email=value
                if key=="owner_no":
                    owner_no=value
                # if key=="comment":
                #     comment=value
                # if key=="city":
                #     city=value
                # if key=="state":
                #     state=value
                # if key=="pincode":
                #     pincode=value
            
            print(owner_name)
            print(uid)
            #query="insert into pet_owner_details values ('{}','{}','{}','{}','{}','{}','{}','{}')".format(14,owner_name,email,owner_name,owner_no,city,state,pincode)
            
            query="insert into pet_owner_details (owner_name,email,owner_no,user_id) values ('{}','{}','{}','{}')".format(owner_name,email,owner_no,uid)
            #('owner_name','email','{city}','{state}','{pincode}','{owner_no}')"
            
            cursor.execute(query)
            cnx.commit()

            query2="select * from pet_owner_details where owner_name='{}' and email='{}'".format(owner_name,email)
            cursor.execute(query2)
            p=list(cursor.fetchall())
            print(p)
            print(p[0][0])
            owner_id=p[0][0]
            # return HttpResponseRedirect('/dashboard')
            #return render(request,'newpetbreed.html')
            
        
        if 'count3' in request.POST:
            cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
            cursor=cnx.cursor()
            a=request.POST
            print(a)
            for key,value in a.items():
                if key=="pet_name":
                    pet_name=value
                if key=="age":
                    age=value
                if key=="color":
                    color=value
                # if key=="comment":
                #     comment=value
                # if key=="city":
                #     city=value
                # if key=="state":
                #     state=value
                # if key=="pincode":
                #     pincode=value
            
            print(pet_name)
            #query="insert into pet_owner_details values ('{}','{}','{}','{}','{}','{}','{}','{}')".format(14,owner_name,email,owner_name,owner_no,city,state,pincode)
            
            query="insert into pet_profile (pet_name,age,color,owner_id) values ('{}','{}','{}','{}')".format(pet_name,age,color,owner_id)
            #('owner_name','email','{city}','{state}','{pincode}','{owner_no}')"
            cursor.execute(query)
            cnx.commit()

            query8 = "select uname from newsigned where user_id='{}'".format(uid)
            cursor.execute(query8)
            usern=list(cursor.fetchall())
            usernew=usern[0][0]
            print(usern)
            print(usernew)

            # return HttpResponseRedirect('/dashboard')
            #return render(request,'newpetbreed.html')
            return redirect('/dashboard',usernew)


        # if 'log' in request.POST:
        #     cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
        #     cursor=cnx.cursor()
        #     d=request.POST
        #     for key,value in d.items():
        #         if key=="logemail":
        #             uname=value
        #         if key=="logpass":
        #             password=value
        #     print('HE'+uname)
        #     query2="select * from signedup where uname='{}' and pass='{}'".format(uname,password)
        #     cursor.execute(query2)
        #     t=tuple(cursor.fetchall())
        #     print(t)
        #     if t==():
        #         return render(request,'mopage.html')
        #     else:
        #         return render(request,'newpetprofile.html')


    

    return render(request,'newpetprofile.html')





def logdashboard(request):
    e=[]
    f=[]
    b2=[]
    newe=0
    pcnew=[]
    cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
    cursor=cnx.cursor()
    query="select * from pet_owner_details where user_id='{}'".format(uid)
    cursor.execute(query)
    p=list(cursor.fetchall())
    print("uid")
    print(uid)
    #print("Mydetail"+ p)
    #print(p[0][0])
    owner_id=p[0][0]

    if 'count3' in request.POST:
        
        a=request.POST
        print(a)
        for key,value in a.items():
            if key=="pet_name":
                pet_name=value
            if key=="age":
                age=value
            if key=="color":
                color=value
            # if key=="comment":
            #     comment=value
            # if key=="city":
            #     city=value
            # if key=="state":
            #     state=value
            # if key=="pincode":
            #     pincode=value
        
        print(pet_name)
        #query="insert into pet_owner_details values ('{}','{}','{}','{}','{}','{}','{}','{}')".format(14,owner_name,email,owner_name,owner_no,city,state,pincode)
        
        query="insert into pet_profile (pet_name,age,color,owner_id) values ('{}','{}','{}','{}')".format(pet_name,age,color,owner_id)
        #('owner_name','email','{city}','{state}','{pincode}','{owner_no}')"
        cursor.execute(query)
        cnx.commit()


    l=p[0][0]
    query2="select * from pet_profile where owner_id='{}'".format(l)
    cursor.execute(query2)
    r=list(cursor.fetchall())
    print(r)

    if 'count4' in request.POST:
        a=request.POST
        print(a)
        # query3="select bar_val from loading ORDER BY id DESC LIMIT 1"
        # cursor.execute(query3)
        # c=list(cursor.fetchall())
        # e=c[0][0]
        # print(e)
        # print(c)

        query3="select Weight from petdaily_activity ORDER BY id DESC LIMIT 1"
        cursor.execute(query3)
        c=list(cursor.fetchall())
        e=c[0][0]
        print(e)
        print(c)
        e=float(e)
        newe = (e*100)/130
        newe = int(newe)
        print(newe)


        query4="select max(Total) from petdaily_activity"
        cursor.execute(query4)
        q=list(cursor.fetchall())
        f=q[0][0]
        print(f)
        print(q)
        
        query5 = "select sum(TempConsumption) from petdaily_activity where Time between '09:00:00' and '11:59:59'"
        cursor.execute(query5)
        b1=list(cursor.fetchall())
        b2=b1[0][0]
        print(b1)
        print(b2)
       
        query6="select max(PetActionCount) from petdaily_activity"
        cursor.execute(query6)
        pc=list(cursor.fetchall())
        pcnew=pc[0][0]
        print(pc)
        print(pcnew)

    # a=[]


    # a.append(p[0][1])
    # a.append(p[0][2])
    # a.append(p[0][7])
    context = {
        'owner_name':p[0][1],
        'email':p[0][2],
        'owner_no':p[0][7],
        # 'pet_name':r[0][1],
        # 'pet_age':r[0][2],
        # 'pet_color':r[0][3]
        'pet_name':r,
        'byval':newe,
        'tot':f,
        'barmy':b2,
        'petact':pcnew
    }
    

    # dataJSON = dumps(context)
    return render(request,'logetprofile.html',context)

def homelog(request):
    global email,password

    if request.method =="POST":
        cnx=sql.connect(host="127.0.0.1",user="root",passwd="sammy1234",database="exampledb")
        cursor=cnx.cursor()
        d=request.POST
        for key,value in d.items():
            if key=="logemail":
                email=value
            if key=="logpass":
                password=value
        
        #query="insert into signedup values ('{}','{}','{}')".format(uname,password,email)
        query="select * from signedup where uname='{}' and pass='{}'".format(email,password)
        cursor.execute(query)
        t=list(cursor.fetchall())
        if t==():
            return render(request,'petlog.html')
        else:
            return render(request,'petlog2.html')

    return render(request,'newpet2.html')

def newhome(request):
    return render(request,'newpet2.html')