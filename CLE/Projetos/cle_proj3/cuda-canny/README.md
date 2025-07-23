## VPN Connection

Before deployment, establish a VPN connection to the UA network:

### Connect to VPN
```bash
snx -s go.ua.pt -u user@ua.pt
```

### Disconnect from VPN
```bash
snx -d
```

## Deployment Process

The `deploy.sh` script automates copying the project folder to the remote server and establishing an SSH connection:

1. Make the deployment script executable:
   ```bash
   chmod +x deploy.sh
   ```

2. Run the deployment script:
   ```bash
   ./deploy.sh
   ```

The script will copy the `cuda-canny` folder to the remote server and automatically log you in.

### How to Exit the Remote Server

After connecting to the remote server through SSH, you can exit the session using any of these methods:

Type `exit` and press Enter:
```bash
exit
```
