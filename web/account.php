<?
  include("utility.inc.php");
  start();

  if($U_ACT == "register"){
    ?>
    <table style="margin-left: auto; margin-right: auto">
      <tr>
        <td style="vertical-align: top">
          <table class="item" style="width: 200pt; margin-left: auto; margin-right: auto">
            <tr>
              <td class="itemhead" style="text-align: center">
                new user account registration
              </td>
            </tr>
            <tr>
              <td class="itembody">
                <form id="addnews" method="post" action="http://sphere.sourceforge.net/accounts.php?U_ACT=add">
                  <div>
                    user name*<br />
                    <input type="text" name="F_USERNAME" style="width: 100%" /><br />
                    password*<br />
                    <input type="text" name="F_PASSWORD" style="width: 100%" /><br />
                    verify password*<br />
                    <input type="text" name="F_VERIFYPASSWORD" style="width: 100%" /><br />
                    password question* (ex: say it, say i'm your momma!)<br />
                    <input type="text" name="F_PASSWORDQUESTION" style="width: 100%" /><br />
                    password answer* (ex: okay, okay! you're my momma!)<br />
                    <input type="text" name="F_PASSWORDANSWER" style="width: 100%" /><br />
                    email address (we wont give this out)*<br />
                    <input type="text" name="F_EMAIL" style="width: 100%" /><br /><br />
                    Personal Crap<br /><br />
                    first name<br />
                    <input type="text" name="F_FIRSTNAME" style="width: 100%" /><br />
                    last name<br />
                    <input type="text" name="F_LASTNAME" style="width: 100%" /><br />
                    year of birth (ex: 1975)<br />
                    <input type="text" name="F_YEAROFBIRTH" style="width: 100%" /><br />
                    month of birth (ex: 09)<br />
                    <input type="text" name="F_MONTHOFBIRTH" style="width: 100%" /><br />
                    day of birth (ex: 02)<br />
                    <input type="text" name="F_DAYOFBIRTH" style="width: 100%" /><br />
                    picture url (60x60 or so)<br />
                    <input type="text" name="F_IMAGE" style="width: 100%" /><br />
                    <a href="javascript:document.getElementById('addnews').submit()">submit</a>
                  </div>
                </form>
              </td>
            </tr>
          </table>
        </td>
        <td style="vertical-align: top">
          <table class="item" style="width: 200pt; margin-left: auto; margin-right: auto">
            <tr>
              <td class="itemhead" style="text-align: center">
                some things to keep in mind
              </td>
            </tr>
            <tr>
              <td class="itembody">
                1) for retrieval purposes, we can view your password. don't choose your normal password if you care. we promise we won't do anything with it, but it's a good idea anyway.<br /><br />
                2) fields marked with '*' are required fields.<br /><br />
                3) if an example is given for a field, do it that way. otherwise the registration script will make you fill this out again.<br /><br />
                4) after you sign up, you can login and edit some things about your account such as the theme you wish to use for the site.<br /><br />
                5) you must be logged in and have an account to post on the forums!<br /><br />
                6) if you want, after you sign up, you can edit a skills list to let other sphere users know what your talents are. this is an easy way for sphere users to gather members for projects or get help. we reccomend you fill this out to help the sphere community grow.<br /><br />
                7) don't be an asshole on the forums, don't choose an incredibly large image (we'll notice that anyway and notify you), etc.<br /><br />
                8) if you ever forget your password, you can always retrieve it by answering your password question correctly. if you still can't even remember your password answer, send us an email and we'll retrieve it for you if we feel it's safe.<br /><br />
                9) if this process takes a while (which it shouldn't), don't hit submit again!
              </td>
            </tr>
          </table>
        </td>
      </tr>
    </table>
    <?
  }
  
  important_links();
?>