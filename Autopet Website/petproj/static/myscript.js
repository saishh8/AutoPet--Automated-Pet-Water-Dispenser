const registerButton = document.getElementById('register')
const loginButton = document.getElementById('login')
const container = document.getElementById('container')

registerButton.onclick = function(){
	 container.className = 'active'
}
loginButton.onclick = function(){
		container.className = 'close'
}



$('reg').click(function(){
	document.getElementById('log1').value='';
	document.getElementById('log2').value='';
	document.getElementById('log3').value='';
})




