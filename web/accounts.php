<?
  include("utility.inc.php");
  start();

  if($U_ACT == "register"){
    ?>
    
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
    
    <?
  }
?>