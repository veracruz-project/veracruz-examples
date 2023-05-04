terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 4.16"
    }
  }

  required_version = ">= 1.2.0"
}

data "aws_ami" "ubuntu" {
  most_recent = true

  filter {
    name   = "name"
    #arm64
    #values = ["ubuntu/images/hvm-ssd/ubuntu-jammy-22.04-arm64-server-*"]
    #x86_64
    values = ["ubuntu/images/hvm-ssd/ubuntu-jammy-22.04-amd64-server-*"]
  }
  owners = ["099720109477"]
}

resource "random_string" "agent_token" {
  length  = 24
  special = false
}

resource "random_string" "k3s_edge_id" {
  length  = 24
  special = false
}

resource "aws_iam_instance_profile" "instance_profile" {
  name  = "${var.deployment_name}-InstanceProfile"
  role  = var.iam_role_name
  count = var.iam_role_name == null ? 0 : 1
}

data "cloudinit_config" "userData" {
  part {
    content      = <<EOF
#cloud-config
---
hostname: "${var.deployment_name}"
EOF
    content_type = "text/cloud-config"
  }

  part {
    content      = <<EOF
#!/bin/bash
echo "----- Installing k3s"
echo K3S_KUBECONFIG_MODE=${var.kubeconfig_mode} K3S_TOKEN=${random_string.agent_token.result} email=${var.letsencrypt_email} > /tmp/variables
curl -sfL https://get.k3s.io | K3S_KUBECONFIG_MODE=${var.kubeconfig_mode} K3S_TOKEN=${random_string.agent_token.result} sh - 
echo "----- updating ubuntu"
apt-get update -y && apt-get upgrade -y && apt-get install awscli git make docker.io -y
usermod -a -G docker ubuntu
echo "----- Adding helm"
curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash
echo "export KUBECONFIG=/etc/rancher/k3s/k3s.yaml" >> /home/ubuntu/.profile
echo "export KUBECONFIG=/etc/rancher/k3s/k3s.yaml" >> /home/ubuntu/.bashrc
echo "----- git clone veracruz-examples"
#sudo su - ubuntu bash -c "git clone https://github.com/veracruz-project/veracruz-examples.git;cd veracruz-examples/i-poc;make veracruz-client;cd main-k3s;make CACert.pem PROGCert.pem"
sudo su - ubuntu bash -c "git clone https://github.com/alexandref75/veracruz-examples.git;cd veracruz-examples;git checkout terraform-helm;cd i-poc;make veracruz-client;cd main-k3s;make CACert.pem PROGCert.pem"
echo "----- Waiting for k3s to start"
until [ -f /etc/rancher/k3s/k3s.yaml ]
do
     sleep 5
done
sudo su - ubuntu bash -c "cd veracruz-examples/i-poc/charts/veracruz-nitro-demo;./helm-script.sh"
EOF
    content_type = "text/x-shellscript"
  }

  part {
    content      = var.manifest_bucket_path == "" ? "" : <<EOF
#!/bin/bash
aws s3 sync s3://${var.manifest_bucket_path} /var/lib/rancher/k3s/server/manifests/
EOF
    content_type = "text/x-shellscript"
  }

#  part {
#    content      = <<EOF
##!/bin/bash
#apt-get update && \
#apt-get install ec2-instance-connect -y
#EOF
#    content_type = "text/x-shellscript"
#  }
}

data "cloudinit_config" "agent_user_data" {
  part {
    content      = <<EOF
#!/bin/bash
yum install -y container-selinux selinux-policy-base aws-nitro-enclaves-cli
sed -i -e "s/memory_mib: 512/memory_mib: 2200/" /etc/nitro_enclaves/allocator.yaml
sysctl -w vm.nr_hugepages=1100
echo "vm.nr_hugepages=1100" > /etc/sysctl.d/99-hugepages.conf
systemctl start nitro-enclaves-allocator.service
systemctl enable nitro-enclaves-allocator.service
curl -sfL https://get.k3s.io | K3S_TOKEN=${random_string.agent_token.result} K3S_URL=https://${aws_instance.k3s_instance.public_dns}:6443 INSTALL_K3S_SELINUX_WARN=true sh -s - --node-label smarter-device-manager=enabled
EOF
    content_type = "text/x-shellscript"
  }
}

data "aws_ami" "amazon_linux" {
  most_recent = true

  filter {
    name   = "name"
    values = ["al2023-ami-*-x86_64"]
  }
  owners = ["137112412989"]
}

resource "aws_key_pair" "k3s_keypair" {
  key_name   = var.deployment_name
  public_key = var.keypair_content
  count      = 1
}

resource "aws_instance" "k3s_instance" {
  ami                         = var.ami_id == null ? data.aws_ami.ubuntu.id : var.ami_id
  associate_public_ip_address = var.assign_public_ip
  instance_type               = var.instance_type
  key_name                    = aws_key_pair.k3s_keypair[0].key_name
  iam_instance_profile        = var.iam_role_name == null ? null : aws_iam_instance_profile.instance_profile[0].name
  subnet_id                   = var.subnet_id == "" ? "" : var.subnet_id
  vpc_security_group_ids      = var.security_group_ids
  user_data                   = data.cloudinit_config.userData.rendered
  tags = {
      Name = "${var.deployment_name}-k3s"
  }
}

resource "aws_instance" "k3s_agent_instance" {
  ami                         = var.ami_id == null ? data.aws_ami.amazon_linux.id : var.ami_id
  associate_public_ip_address = var.assign_public_ip
  instance_type               = var.agent_instance_type
  key_name                    = aws_key_pair.k3s_keypair[0].key_name
  iam_instance_profile        = var.iam_role_name == null ? null : aws_iam_instance_profile.instance_profile[0].name
  subnet_id                   = var.subnet_id == "" ? "" : var.subnet_id
  vpc_security_group_ids      = var.security_group_ids
  user_data                   = data.cloudinit_config.agent_user_data.rendered
  enclave_options {
      enabled = true
  }
  tags = {
      Name = "${var.deployment_name}-k3sNode"
  }
}

output "agent_instance" {
  value = aws_instance.k3s_agent_instance
}

output "instance" {
  value = aws_instance.k3s_instance
}

output "k3s_edge" {
  value = random_string.k3s_edge_id
}
