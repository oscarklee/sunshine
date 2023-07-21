function statusChangeCallback(response) {
    console.info(response);
    if (response.status === 'connected') {
        document.querySelector("#fb-access-token").value = response.authResponse.accessToken;
        document.querySelector("#fb-login").hidden = true;
        document.querySelector("#fb-logout").hidden = false;
    } else {
        document.querySelector("#fb-login").hidden = false;
        document.querySelector("#fb-logout").hidden = true;
    }
}

function logOut() {
    FB.logout(function(response) {
        statusChangeCallback(response);
    });
}

function logIn() {
    FB.login(function(response){
        statusChangeCallback(response);
    }, {scope: 'public_profile,email'});
}

window.fbAsyncInit = function() {
    FB.init({
      appId      : '1358849168232954',
      cookie     : true,
      xfbml      : true,
      version    : 'v17.0'
    });


    FB.getLoginStatus(function(response) {
      statusChangeCallback(response);
    });
};