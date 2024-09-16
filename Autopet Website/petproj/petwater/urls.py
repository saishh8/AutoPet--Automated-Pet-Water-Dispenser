from django.urls import path
from . import views

urlpatterns = [
    #path('', views.home),
    path('', views.home,name="dash"),
    path('dashboard', views.logdashboard,name="dashboard"),
    path('newdashboard', views.dashboard,name="dashboard")
    # ,name='dash'

]