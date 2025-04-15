<div class="login-container">
    <div class="card login-card">
        <h2>Anmelden</h2>
        <form id="login-form" class="login-form">
            <div class="form-group">
                <label for="username">Benutzername</label>
                <input type="text" id="username" name="username" required>
            </div>
            
            <div class="form-group">
                <label for="password">Passwort</label>
                <input type="password" id="password" name="password" required>
            </div>
            
            <div class="form-group">
                <button type="submit" class="button login-button">Anmelden</button>
            </div>
            
            <div id="login-error" class="error-message" style="display: none;"></div>
        </form>
    </div>
</div>

<script>
document.addEventListener('DOMContentLoaded', function() {
    const loginForm = document.getElementById('login-form');
    const loginError = document.getElementById('login-error');
    
    loginForm.addEventListener('submit', function(event) {
        event.preventDefault();
        
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;
        
        if (!username || !password) {
            showLoginError('Bitte geben Sie Benutzername und Passwort ein.');
            return;
        }
        
        // Login-Button deaktivieren und Ladezustand anzeigen
        const loginButton = document.querySelector('.login-button');
        loginButton.disabled = true;
        loginButton.textContent = 'Anmeldung läuft...';
        
        // Anmeldedaten an die API senden
        fetch('?action=api&endpoint=login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                username: username,
                password: password
            })
        })
        .then(response => response.json())
        .then(data => {
            // Login-Button zurücksetzen
            loginButton.disabled = false;
            loginButton.textContent = 'Anmelden';
            
            if (data.status === 'ok') {
                // Token speichern
                localStorage.setItem('api_token', data.token);
                
                // Zur Dashboard-Seite weiterleiten
                window.location.href = '?action=dashboard';
            } else {
                showLoginError(data.message || 'Anmeldung fehlgeschlagen.');
            }
        })
        .catch(error => {
            // Login-Button zurücksetzen
            loginButton.disabled = false;
            loginButton.textContent = 'Anmelden';
            
            console.error('Fehler bei der Anmeldung:', error);
            showLoginError('Verbindungsfehler. Bitte versuchen Sie es später erneut.');
        });
    });
    
    // Fehlermeldung anzeigen
    function showLoginError(message) {
        loginError.textContent = message;
        loginError.style.display = 'block';
    }
});
</script>

<style>
.login-container {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 70vh;
    padding: var(--spacing-md);
}

.login-card {
    width: 100%;
    max-width: 400px;
}

.login-form {
    margin-top: var(--spacing-lg);
}

.error-message {
    color: var(--error-color);
    padding: var(--spacing-md);
    margin-top: var(--spacing-md);
    background-color: rgba(233, 69, 47, 0.1);
    border-radius: var(--border-radius-sm);
}

.login-button {
    width: 100%;
    padding: var(--spacing-md);
}
</style>